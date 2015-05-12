//
//  net_msgs.c
//  SDL_net
//
//  Created by cristian araya on 06/05/15.
//  Copyright (c) 2015 project. All rights reserved.
//
#ifdef __APPLE__
#include "SDL2_net/SDL_net.h"

#elif __linux
#include "SDL2/SDL_net.h"
#endif

#include "server_structs.h"
#include "net_msgs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>


#define N_CLIENTS 2
msg_stack recvPool;
pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;

void poolInit(){
    recvPool.size=0;
}

int Converter_BytesToInt32(char data[], int* index){
    
    int value;
    
    value = ((int)data[*index] << 24) + ((int)data[*index + 1] << 16)
    + ((int)data[*index + 2] << 8) + ((int)data[*index + 3]);
    
    (*index) += 4;
    
    return value;
}

int Converter_Int32ToBytes(char data[], int* size, int value)
{
    data[*size] = value >> 24;
    data[*size + 1] = value >> 16;
    data[*size + 2] = value >> 8;
    data[*size + 3] = value;
    (*size) += 4;
    
    return 0;
}

int AddToPool(char* msg) // Funktion fˆr att l‰gga till meddelanden i stacks ( pools).
{
    
    pthread_mutex_lock(&pool_mutex);
    memcpy(recvPool.queue[recvPool.size], msg, 512);
    recvPool.size++;
    pthread_mutex_unlock(&pool_mutex);
    return 0;
}

void SendObjectPos(int objId, int x, int y, int angle)
{
    char msg[512];
    int index=1, i;
    msg[0]=2;
    Converter_Int32ToBytes(msg, &index, objId);
    Converter_Int32ToBytes(msg, &index, x);
    Converter_Int32ToBytes(msg, &index, y);
    Converter_Int32ToBytes(msg, &index, angle);
    
    for(i=0;i<N_CLIENTS;i++)
    {
        if(client[i].status == true)
            SDLNet_TCP_Send(client[i].socket, msg, 512);
    }
    
}

void SendNewObject(int objId, int x, int y, objectType_t type)
{
    char msg[512];
    int index=1, i;
    msg[0]=5;
    Converter_Int32ToBytes(msg, &index, objId);
    Converter_Int32ToBytes(msg, &index, x);
    Converter_Int32ToBytes(msg, &index, y);
    msg[index++]=type;
    
    for(i=0;i<N_CLIENTS;i++)
    {
        if(client[i].status == true)
            SDLNet_TCP_Send(client[i].socket, msg, 512);
    }
    
}

void SendRemoveObject(int objId)
{
    char msg[512];
    int index=1, i;
    msg[0]=6;
    Converter_Int32ToBytes(msg, &index, objId);
    
    for(i=0;i<N_CLIENTS;i++)
    {
        if(client[i].status == true)
            SDLNet_TCP_Send(client[i].socket, msg, 512);
    }
    
}

void SendPlayerId(int PlayerId){
    char msg[512];
    int index=1, i;
    msg[0]=7;
    Converter_Int32ToBytes(msg, &index, PlayerId);
    for(i=0;i<N_CLIENTS;i++)
    {
        if(client[i].status == true)
            SDLNet_TCP_Send(client[i].socket, msg, 512);
    }
    
}

void RecvPlayerMove(int PlayerId, char vertical, char horizontal){
    
}