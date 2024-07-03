import {BuzzerContainer} from "@/app/game/buzzer/buzzer-container";
import {Divider} from "@nextui-org/divider";
import {ManagementContainer} from "@/app/game/management/management-container";
import {BuzzerType} from "@/app/game/types/game-types";

export function Activity(){

    let buzzers: BuzzerType[] = [
        {
            buzzerId: 0,
            buzzerName: 'First Buzzer',
            isPressed: false,
            isLocked: false,
            delay: 2.56
        },
        {
            buzzerId: 1,
            buzzerName: 'Second Buzzer',
            isPressed: false,
            isLocked: false,
            delay: null
        }
    ]

    return (
        <>
            <BuzzerContainer buzzers={buzzers}></BuzzerContainer>
            <Divider className={"mt-5 mb-5"}></Divider>
            <ManagementContainer></ManagementContainer>
        </>
    );
}