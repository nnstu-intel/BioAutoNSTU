import React from 'react';
import { Card } from 'antd';
import Text from 'antd/lib/typography/Text';

import { Device } from 'reducers/devices-reducer';

interface Props {
    device: Device;
    active: boolean;
}

export default function DeviceItem(props: Props): JSX.Element {
    const { device, active } = props;
    const { id, name } = device;

    const style: React.CSSProperties = active ? {
        width: '100%',
        background: 'aliceblue',
    } : {
            width: '100%',
        };

    return (
        <Card.Grid style={style} className='device-item' >
            <Text strong>{id}</Text>
            <Text>{` ${name}`}</Text>
        </Card.Grid>
    );
}
