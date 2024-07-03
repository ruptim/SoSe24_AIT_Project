import {Card, CardBody, CardHeader} from "@nextui-org/card";
import {Chip} from "@nextui-org/chip";
import moment from "moment";
import {BuzzerType} from "@/app/game/types/game-types";

type BuzzerParams = {
    buzzer: BuzzerType
}

export function Buzzer({buzzer}:BuzzerParams){
    let lockedClassName: string = buzzer.isLocked ? 'border-3 border-yellow-500' : '';

    return (
        <Card>
            <CardHeader className="pb-0 pt-2 px-4 flex-col items-start">
                <p className="text-tiny uppercase font-bold">#{buzzer.buzzerId}</p>
                <small className="text-default-500">{buzzer.buzzerName}</small>
                <small className="text-default-500">
                    {buzzer.delay?.toFixed(3)} {buzzer.delay?('s'):(<>&nbsp;</>)}
                </small>
            </CardHeader>
            <CardBody>
                <div className={"flex justify-center"}>
                    {buzzer.isPressed ? (<Chip className={`min-w-full text-center ${lockedClassName}`} color={"success"}>pressed</Chip>): (<Chip className={`min-w-full text-center ${lockedClassName}`} color={"default"}>not pressed</Chip>)}
                </div>
            </CardBody>
        </Card>
    )
}