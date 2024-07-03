'use client';

import {useEffect, useState} from "react";
import {Card, CardBody, CardHeader} from "@nextui-org/card";
import {Chip} from "@nextui-org/chip";

type BuzzerParams = {
    buzzerId: number;
    buzzerName: string;
    isPressed: boolean;
    isLocked: boolean;
    delay: number | null;
}

export function Buzzer({buzzerId, buzzerName, isLocked, isPressed, delay}:BuzzerParams){
    let lockedClassName: string = isLocked ? 'border-3 border-yellow-500' : '';
    // let pressedColorString: "success" | "default" | "primary" | "secondary" | "warning" | "danger" | undefined = isPressed ? 'success' : 'default';
    let pressedColorString: string = isPressed ? 'bg-success' : 'bg-default';

    return (
        <Card>
            <CardHeader className="pb-0 pt-2 px-4 flex-col items-start">
                <p className="text-tiny uppercase font-bold">#{buzzerId}</p>
                <small className="text-default-500">{buzzerName}</small>
                <small className="text-default-500">
                    {delay?.toFixed(3)} {delay?('s'):(<>&nbsp;</>)}
                </small>
            </CardHeader>
            <CardBody>
                <div className={"flex justify-center"}>
                    <Chip className={`min-w-full text-center ${lockedClassName} ${pressedColorString}`}>{isPressed ? '' : 'not'} pressed</Chip>
                </div>
            </CardBody>
        </Card>
    )
}