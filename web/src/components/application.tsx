import React from 'react';
import { Row, Col } from 'antd';

import DevicesList from './devices-list';
import UsersList from './users-list';

export default function Application(): JSX.Element {
    return (
        <Row>
            <Col span={8} offset={1} className='devices-list'>
                <DevicesList />
            </Col>
            <Col span={12} offset={1} className='users-list'>
                <UsersList />
            </Col>
        </Row>
    );
}
