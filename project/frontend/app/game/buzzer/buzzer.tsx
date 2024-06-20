import {Card, CardBody, CardHeader} from "@nextui-org/card";
import {Chip} from "@nextui-org/chip";

type BuzzerParams = {
    buzzerId: number,
    buzzerName: string,
    isPressed: boolean
}

export function Buzzer({buzzerId, buzzerName, isPressed}:BuzzerParams){

    return (
        <Card>
            <CardHeader className="pb-0 pt-2 px-4 flex-col items-start">
                <p className="text-tiny uppercase font-bold">#{buzzerId}</p>
                <small className="text-default-500">{buzzerName}</small>
            </CardHeader>
            <CardBody>
                <div className={"flex justify-center"}>
                    {isPressed ? (<Chip color={"success"}>pressed</Chip>): (<Chip color={"default"}>not pressed</Chip>)}
                </div>
            </CardBody>
        </Card>
    )
}