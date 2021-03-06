#ifdef __APPLE__
#include "SDL2/SDL.h"
#include "SDL2_net/SDL_net.h"

#elif __linux
#include "SDL2/SDL.h"
#include "SDL2/SDL_net.h"
#endif

#include <stdbool.h>
#include <pthread.h>

#include "client_handler.h"
#include "server_structs.h"
#include "client_process.h"
#include "spel_objects.h"
#include "net_msgs.h"


//Function to handle incomming connections.
void* client_handle(void* objs)
{
    TCPsocket sd, csd, tmp; /* Socket descriptor, Client socket descriptor */
    IPaddress ip, *remoteIP;
    int listening = 1;
    int sockets_available=1;

    Scene *level = (Scene*)objs;// = *(GameObject*) objs;
    GameObject player;

    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Resolving the host using NULL make network interface to listen */
    if (SDLNet_ResolveHost(&ip, NULL, 2000) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Open a connection with the IP provided (listen on the host's port) */
    if (!(sd = SDLNet_TCP_Open(&ip)))
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    while (listening)
    {
        /* Our server should always accept new connections but close them if server full */
        if ((csd = SDLNet_TCP_Accept(sd)))
        {
            /* Now we can communicate with the client using csd socket
             * sd will remain opened waiting other connections */

            /* Get the remote address */
            if ((remoteIP = SDLNet_TCP_GetPeerAddress(csd)))
            /* Print the address, converting in the host format */
                printf("Host connected: %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
            else
                fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());

            if (sockets_available)
            {
                unsigned char msg[128] = {0};
                for(int i=0; i<N_CLIENTS;i++)
                {
                    if(client[i].status == false)
                    {
                        printf("Client accepted at index %d\n", i);
                        client[i].socket = csd;
                        client[i].status = true;

                        //Waiting until a player name message is received.
                        while (msg[0]!= NET_PLAYER_NAME)
                        {
                            SDLNet_TCP_Recv(client[i].socket, msg, 128);//int nameLenght, str name
                        }

                        int index = 1;
                        int nameLength = Converter_BytesToInt32(msg, &index);

                        //printf("Length = '%d'\n", nameLength);
                        for (int j = 0; j<nameLength; j++)
                        {
                            client[i].name[j] = (char) msg[j+index];
                        }

                        for(int j = 0; j< N_CLIENTS;j++)
                        {
                            if(!strcmp(client[j].name, client[i].name) && i != j)
                            {
                                //client[i].name[nameLength];
                                sprintf(client[i].name, "%s%d",client[i].name,i);
                                break;
                            }

                        }

                        index+= nameLength;

                        printf("__________________\n");
                        printf("Player '%s' connected.\n", client[i].name);
                        player = CreatePlayer(2750, 5350, level->nextId++, client[i].name);

                        printf("Player object id = %d\n", player.obj_id);
                        client[i].playerId = player.obj_id;//+1500;

                        printf("Player class = %d\n", msg[index]);
                        printf("__________________\n");


                        SendPlayerId(client[i].playerId);

                        AddObject(level, player, false);

                        client[i].pClass = msg[index];

                        for(int j = 0; j < N_CLIENTS ; j++) //Sends the player to the other clients in the lobby.
                        {
                            if(client[j].status)
                            {
                                SendLobbyPlayer(client[j].name, client[j].pClass, client[j].playerId);
                            }
                        }

                        pthread_create(&client[i].tid, NULL, &client_process, &i); //Thread that handles the clients messages
                        break;//found a empty space
                    }

                }
            }

            else
            {
                tmp = csd;
                printf("Server full. Connection denied\n");
                SDLNet_TCP_Send(tmp, "Server full. Connection denied", 30);
                SDLNet_TCP_Send(tmp, "Turn you off baby", 20);
                SDLNet_TCP_Close(tmp);
            }
        }

    }


    //SDLNet_TCP_Close(csd);
    SDLNet_TCP_Close(sd);
    SDLNet_Quit();

    return EXIT_SUCCESS;
}
