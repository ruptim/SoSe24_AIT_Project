"use client";

import { Divider } from "@nextui-org/divider";
import { useState } from "react";

import { BuzzerContainer } from "@/app/game/buzzer/buzzer-container";
import { ManagementContainer } from "@/app/game/management/management-container";
import { BuzzerType } from "@/app/game/types/game-types";
import {socket} from "@/app/socket";
import {backendConfig} from "@/config/backend-config";

type ActivityParams = {
  buzzers: BuzzerType[],
};

export function ActivityContainer({ buzzers}: ActivityParams) {
  const [isLocked, setLocked] = useState(false);
  // const [buzzerArr, setBuzzerArr] = useState<BuzzerType[]>(buzzerList);

  function handleResetClick() {
    // buzzerArr.map((buzzer) => (buzzer.isPressed = false));
    // setBuzzerArr([...buzzerArr]);

    console.log('Send reset');
    socket.emit(backendConfig.events.reset);
  }

  function handleLockClick() {
    // let newLocked = !isLocked;

    // setLocked(newLocked);
    // buzzerArr.map((buzzer) => (buzzer.isLocked = newLocked));
    // setBuzzerArr(buzzerArr);
    console.log('Send lock');
    socket.emit(backendConfig.events.lock);
  }

  function handleBuzzerDelete( buzzer: BuzzerType){
    console.log('Send remove for ' + buzzer);
    socket.emit(backendConfig.events.remove, buzzer);
  }

  return (
    <div>
      <BuzzerContainer
        buzzers={buzzers}
        isAllLocked={isLocked}
        onLockClick={handleLockClick}
        onResetClick={handleResetClick}
      />
      <Divider className={"mt-5 mb-5"} />
      <ManagementContainer buzzers={buzzers} onBuzzerDelete={handleBuzzerDelete}/>
    </div>
  );
}
