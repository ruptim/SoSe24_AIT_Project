'use client';

import {Card, CardBody, CardHeader} from "@nextui-org/card";
import {Chip} from "@nextui-org/chip";

type BuzzerParams = {
    buzzerRank: number | null;
    buzzerName: string;
    isPressed: boolean;
    isLocked: boolean;
    delay: number | null;
}

export function Buzzer({buzzerRank, buzzerName, isLocked, isPressed, delay}:BuzzerParams){
    return (
        <Card>
            <CardHeader className="pb-0 pt-2 px-4 flex-col items-start">
                <p className="text-tiny uppercase font-bold">{buzzerName}</p>
                <small className="text-default-500">{buzzerRank ? (`#${buzzerRank}`) : (<>&nbsp;</>)}</small>
                <small className="text-default-500">
                    {delay?.toFixed(3)} {delay?('s'):(<>&nbsp;</>)}
                </small>
            </CardHeader>
            <CardBody>
                <div className={"flex justify-center"}>
                    <Chip className={`min-w-full text-center ${isLocked ? 'border-3 border-yellow-500' : ''}`} color={isPressed ? "success" : "default"}>{isPressed ? '' : 'not'} pressed</Chip>
                </div>
            </CardBody>
        </Card>
    )
}