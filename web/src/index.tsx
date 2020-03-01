import 'antd/dist/antd.css';
import 'styles.scss';

import React from 'react';
import ReactDOM from 'react-dom';
import { Provider } from 'react-redux';

import Application from './components/application';
import getStore from './store';

const root = window.document.getElementById('react-root');

if (root) {
    const resize = () => {
        root.style.width = `${window.innerWidth}px`;
        root.style.height = `${window.innerHeight}px`;
    };
    window.document.addEventListener('resize', resize);
    resize();

    ReactDOM.render(
        (
            <Provider store={getStore()}>
                <Application />
            </Provider >
        ),
        root
    );

}
