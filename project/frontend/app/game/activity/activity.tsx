'use client';

import {BuzzerContainer} from "@/app/game/buzzer/buzzer-container";
import {Divider} from "@nextui-org/divider";
import {ManagementContainer} from "@/app/game/management/management-container";
import {BuzzerType} from "@/app/game/types/game-types";
import {useState} from "react";

let buzzers: BuzzerType[] = [
    {
        buzzerId: 0,
        buzzerName: 'First Buzzer',
        isPressed: true,
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

    const [isLocked, setLocked] = useState(false)

    function handleResetClick(){
        console.log('RESET');
        buzzers.map(buzzer => buzzer.isPressed = false);
    }

    function handleLockClick(){
        console.log('LOCK');
        let newLocked = !isLocked
        setLocked(newLocked);
        buzzers.map(buzzer => buzzer.isLocked = newLocked);
    }

    return (
        <>
            <BuzzerContainer buzzers={buzzers} onResetClick={handleResetClick} onLockClick={handleLockClick} isAllLocked={isLocked}></BuzzerContainer>
            <Divider className={"mt-5 mb-5"}></Divider>
            <ManagementContainer></ManagementContainer>
        </>
    );
}