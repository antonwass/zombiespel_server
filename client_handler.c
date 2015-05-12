//
//  client_handler.c
//  SDL_net
//
//  Created by cristian araya on 08/05/15.
//  Copyright (c) 2015 project. All rights reserved.
//

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include "SDL2_net/SDL_net.h"

#elif __linux
#include "SDL2/SDL_net.h"
#endif

#include <stdbool.h>
#include <pthread.h>

#include "client_handler.h"
#include "server_structs.h"
#include "client_process.h"
#include "spel_objects.h"

#define N_CLIENTS 2


void* client_handle(void* objs){
    TCPsocket sd, csd, tmp; /* Socket descriptor, Client socket descriptor */
    IPaddress ip, *remoteIP;
    int listening = 1, i=0, id=0, objectCount=0;
    int sockets_available=1;
    GameObject objects[100]; //= *(GameObject*) objs;
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
                for(i=0; i<N_CLIENTS;i++){
                    if(client[i].status == false){
                        client[i].socket = csd;
                        client[i].status = true;
                        player = CreatePlayer(0,0, id++);
                        AddObject(objects, player, &objectCount);
                        pthread_create(&client[i].tid, NULL, &client_process, &i);
                        break;//hittat en ledig plats
                    }
                    else
                        continue;
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