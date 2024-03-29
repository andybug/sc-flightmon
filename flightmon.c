#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>

#include <windows.h>

#include "SimConnect.h"

enum REQUEST_ID {
    REQUEST_POSITION_UPDATE,
};

enum DEFINITION_ID {
    DEFINITION_POSITION_UPDATE,
};

struct position_update {
    double latitude;
    double longitude;
    double ground_speed;
    double heading;
    int32_t altitude; // have to watch out for struct packing
};

static volatile sig_atomic_t done = 0;

static void get_iso_8601_timestamp(char* buf, size_t len)
{
    SYSTEMTIME utc;

    GetSystemTime(&utc);
    snprintf(buf, len, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             utc.wYear, utc.wMonth, utc.wDay,
             utc.wHour, utc.wMinute, utc.wSecond, utc.wMilliseconds);
}

static void CALLBACK sc_dispatch_proc(SIMCONNECT_RECV *data, DWORD size, void *context)
{
    switch (data->dwID) {
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        SIMCONNECT_RECV_SIMOBJECT_DATA *object_data = (SIMCONNECT_RECV_SIMOBJECT_DATA*) data;

        if (object_data->dwRequestID == REQUEST_POSITION_UPDATE) {
            const position_update* position = (position_update *) &object_data->dwData;

            const size_t timestamp_size = 64;
            char timestamp[timestamp_size];
            get_iso_8601_timestamp(timestamp, timestamp_size);

            printf("%s, %f, %f, %d, %0.1f, %0.1f\n",
                   timestamp,
                   position->latitude,
                   position->longitude,
                   position->altitude,
                   position->ground_speed,
                   position->heading);
        }
        break;
    }
    case SIMCONNECT_RECV_ID_QUIT:
        fprintf(stderr, "SimConnect quit message received\n");
        break;
    }
}

static void sc_connection(HANDLE handle)
{
    fprintf(stderr, "SimConnect connected\n");

    SimConnect_AddToDataDefinition(handle, DEFINITION_POSITION_UPDATE, "PLANE LATITUDE", "degrees");
    SimConnect_AddToDataDefinition(handle, DEFINITION_POSITION_UPDATE, "PLANE LONGITUDE", "degrees");
    SimConnect_AddToDataDefinition(handle, DEFINITION_POSITION_UPDATE, "GROUND VELOCITY", "knots");
    SimConnect_AddToDataDefinition(handle, DEFINITION_POSITION_UPDATE, "PLANE HEADING DEGREES GYRO", "degrees");
    SimConnect_AddToDataDefinition(handle, DEFINITION_POSITION_UPDATE, "PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_INT32);

    SimConnect_RequestDataOnSimObject(handle, REQUEST_POSITION_UPDATE, DEFINITION_POSITION_UPDATE, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND);

    while (!done) {
        auto result = SimConnect_CallDispatch(handle, sc_dispatch_proc, NULL);

        if (result != S_OK) {
            fprintf(stderr, "SimConnect dispatch failed\n");
        }

        Sleep(100);
    }

    SimConnect_Close(handle);

    fprintf(stderr, "SimConnect connection closed\n");
}

static void signal_handler(int signum)
{
    fprintf(stderr, "Signal caught, exiting...\n");
    done = 1;
}

int main()
{
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    HANDLE sc_handle = NULL;

    while (true) {
        while (!done && SimConnect_Open(&sc_handle, "SimConnect Agent", NULL, 0, 0, 0) != S_OK) {
            fprintf(stderr, "Failed to open SimConnect, retrying...\n");
            Sleep(1000);
        }

        if (done) {
            break;
        }

        sc_connection(sc_handle);
    }

    return 0;
}
