import {Card, CardBody, CardHeader} from "@nextui-org/card";
import {Chip} from "@nextui-org/chip";
import moment from "moment";

type BuzzerParams = {
    buzzerId: number,
    buzzerName: string,
    timestamp: Date,
    isPressed: boolean,
    isLocked: boolean
}

export function Buzzer({buzzerId, buzzerName, timestamp, isPressed, isLocked}:BuzzerParams){
    let timeStampFormatted: string = moment(timestamp).format('s,SSS');
    let lockedClassName: string = isLocked ? 'border-3 border-yellow-500' : '';

    return (
        <Card>
            <CardHeader className="pb-0 pt-2 px-4 flex-col items-start">
                <p className="text-tiny uppercase font-bold">#{buzzerId}</p>
                <small className="text-default-500">{buzzerName}</small>
                <small className="text-default-500">{timeStampFormatted}s</small>
            </CardHeader>
            <CardBody>
                <div className={"flex justify-center"}>
                    {isPressed ? (<Chip className={`min-w-full text-center ${lockedClassName}`} color={"success"}>pressed</Chip>): (<Chip className={`min-w-full text-center ${lockedClassName}`} color={"default"}>not pressed</Chip>)}
                </div>
            </CardBody>
        </Card>
    )
}