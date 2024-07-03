'use client';

import {BuzzerContainer} from "@/app/game/buzzer/buzzer-container";
import {Divider} from "@nextui-org/divider";
import {ManagementContainer} from "@/app/game/management/management-container";
import {BuzzerType} from "@/app/game/types/game-types";

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

export function Activity(){

    function handleResetClick(){
        console.log('RESET');
    }

    function handleLockClick(){
        console.log('LOCK');
    }

    return (
        <>
            <BuzzerContainer buzzers={buzzers} onResetClick={handleResetClick} onLockClick={handleLockClick}></BuzzerContainer>
            <Divider className={"mt-5 mb-5"}></Divider>
            <ManagementContainer></ManagementContainer>
        </>
    );
}