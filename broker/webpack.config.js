const path = require('path');

module.exports = {
    mode: 'production',
    target: 'node',
    entry: './src/index.ts',
    module: {
        rules: [{
            test: /\.tsx?$/,
            use: 'babel-loader',
            exclude: /node_modules/,
        }],
    },
    resolve: {
        extensions: [ '.tsx', '.ts', '.js' ],
    },
    output: {
        filename: 'communication-server.js',
        path: path.resolve(__dirname, 'dist'),
    },
}