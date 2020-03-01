import React from 'react';
import { Card, Row, Col, Tooltip } from 'antd';
import { SyncOutlined, DeleteOutlined } from '@ant-design/icons';
import Text from 'antd/lib/typography/Text';

import { User } from 'reducers/users-reducer';

interface Props {
    user: User;
}

export default function UserItem(props: Props): JSX.Element {
    const { user } = props;
    const {
        firstname,
        secondname,
        patronymic,
        passport,
        id,
    } = user;

    const style: React.CSSProperties = {
        width: '100%',
    }

    return (
        <Card.Grid style={style} className='user-item' >
            <Row>
                <Col span={22}>
                    <Text strong>{id}</Text>
                    <Text>{` ${secondname}`}</Text>
                    <Text>{` ${firstname}`}</Text>
                    {patronymic && <Text>{` ${patronymic}`}</Text>}
                    <br />
                    <Text strong>Passport number:</Text>
                    <Text>{` ${passport}`}</Text>
                </Col>
                <Col span={2}>
                    <Tooltip title='Update user data'>
                        <SyncOutlined className='update-user-icon' />
                    </Tooltip>
                    <Tooltip title='Delete the user'>
                        <DeleteOutlined className='delete-user-icon' />
                    </Tooltip>
                </Col>
            </Row>
        </Card.Grid>
    );
}
