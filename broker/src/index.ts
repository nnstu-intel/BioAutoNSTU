import WebSocket from 'ws';
import winston from 'winston';

import MessageHandler from './message-handler';

winston.configure({
    level: 'silly',
    transports: [
        new winston.transports.Console({
            format: winston.format.combine(
                winston.format.timestamp(),
                winston.format.prettyPrint(),
                winston.format.colorize({colors: {
                    silly: 'blue',
                    debug: 'magenta',
                    verbose: 'green',
                    info: 'cyan',
                    warn: 'yellow',
                    error: 'red'
                }, all: true}),
            ),
        }),
        new winston.transports.File({
            format: winston.format.combine(
                winston.format.timestamp(),
                winston.format.prettyPrint(),
            ),
            filename: 'server_logs.log',
        })
    ]
});

const ENV_PORT = 'PORT' in process.env ? process.env.PORT : null;
let PORT = 8080;
if (ENV_PORT && !Number.isNaN(+ENV_PORT)) {
    PORT = +ENV_PORT;
}

try {

    const server = new WebSocket.Server({
        port: PORT,
    });

    server.on('connection', (socket: WebSocket) => {
        winston.verbose('Socket connection has been established');

        const messageHandler = new MessageHandler();
        const handleMessage = (message: WebSocket.Data) => {
            messageHandler.handle(socket, message);
        };

        socket.on('message', handleMessage);
        socket.once('close', () => {
            winston.verbose('Socket connection has been closed');
            socket.off('message', handleMessage);
        });
    });

    winston.info(`Server has started listening port ${PORT}`);
} catch (error) {
    winston.error(error.toString());
    process.exit(1);
}
