import WebSocket from 'ws';

export enum InboundMessageTypes {
    CONNECT_PI = 'CONNECT_PI',
    CONNECT_WEB_CLIENT = 'CONNECT_WEB_CLIENT',
    UPDATE_PI_USERS = 'UPDATE_PI_USERS',
    AUTHORIZE_CLIENT = 'AUTHORIZE_CLIENT',
    CREATE_PI_USER = 'CREATE_PI_USER',
    REMOVE_PI_USER = 'REMOVE_PI_USER',
    OK_STATUS = 'OK_STATUS',
    ERROR_STATUS = 'ERROR_STATUS',
}

export enum OutboundMessageTypes {
    CREATE_PI_USER = 'CREATE_PI_USER',
    REMOVE_PI_USER = 'REMOVE_PI_USER',
    UPDATE_PI_DEVICES = 'UPDATE_PI_DEVICES',
    UPDATE_PI_USERS = 'UPDATE_PI_USERS',
    OK_STATUS = 'OK_STATUS',
    ERROR_STATUS = 'ERROR_STATUS',
}

export interface User {
    id?: number;
    passport: string;
    firstname: string;
    secondname: string;
    patronymic?: string;
    image?: string; // base64 encoded jpeg
}

export interface WebClient {
    id: number;
    socket: WebSocket;
    connectedPIDevices: number[];
}

export interface PIDevice {
    id: number;
    pin: string;
    name: string;
    users: User[];
    socket: WebSocket;
}

export interface ConnectPIPayload { // InboundMessageTypes.CONNECT_PI
    name: string;
    pin: string;
}

export interface UpdatePIUsersPayload { // InboundMessageTypes.UPDATE_PI_USERS (without deviceID), // OutboundMessageTypes.UPDATE_PI_USERS
    deviceID?: number;
    users: User[];
}

export interface AuthorizeClientPayload { // InboundMessageTypes.AUTHORIZE_CLIENT
    deviceID: number;
    pin: string;
}

export interface CreatePIUserPayload { // InboundMessageTypes.CREATE_PI_USER, // OutboundMessageTypes.CREATE_PI_USER (without deviceID)
    deviceID?: number;
    user: User;
}

export interface RemovePIUserPayload { // InboundMessageTypes.REMOVE_PI_USER, // OutboundMessageTypes.REMOVE_PI_USER (without deviceID)
    deviceID?: number;
    userID: number;
}

export interface InboundMessage<PayloadType> {
    type: InboundMessageTypes,
    payload: PayloadType;
}

export interface UpdatePIDevicesPayload { // OutboundMessageTypes.UPDATE_PI_DEVICES
    devices: {
        id: number;
        name: string;
    }[];
}

export interface OutboundMessage<PayloadType> {
    type: OutboundMessageTypes;
    payload: PayloadType;
}

export interface OKStatusPayload { // InboundMessageTypes.OK_STATUS (for: OutboundMessageTypes), // OutboundMessageTypes.OK_STATUS (for: InboundMessageTypes)
    for: InboundMessageTypes | OutboundMessageTypes;
}

export interface ErrorStatusPayload {
    for: InboundMessageTypes | OutboundMessageTypes | 'UNKNOWN';
}
