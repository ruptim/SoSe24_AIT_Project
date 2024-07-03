'use client';

import {BuzzerContainer} from "@/app/game/buzzer/buzzer-container";
import {Divider} from "@nextui-org/divider";
import {ManagementContainer} from "@/app/game/management/management-container";
import {BuzzerType} from "@/app/game/types/game-types";
import {useState} from "react";

type ActivityParams = {
    buzzerList: BuzzerType[]
}

export function ActivityContainer({buzzerList}:ActivityParams){

    const [isLocked, setLocked] = useState(false)
    const [buzzerArr, setBuzzerArr] = useState<BuzzerType[]>(buzzerList);

    function handleResetClick(){
        buzzerArr.map(buzzer => buzzer.isPressed = false);
        setBuzzerArr([... buzzerArr]);
    }

    function handleLockClick(){
        let newLocked = !isLocked
        setLocked(newLocked);
        buzzerArr.map(buzzer => buzzer.isLocked = newLocked);
        setBuzzerArr(buzzerArr);
    }

    return (
        <>
            <BuzzerContainer buzzers={buzzerArr} onResetClick={handleResetClick} onLockClick={handleLockClick} isAllLocked={isLocked}></BuzzerContainer>
            <Divider className={"mt-5 mb-5"}></Divider>
            <ManagementContainer></ManagementContainer>
        </>
    );
}