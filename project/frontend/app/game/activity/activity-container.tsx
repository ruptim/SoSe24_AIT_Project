"use client";

import { Divider } from "@nextui-org/divider";
import { useState } from "react";

import { BuzzerContainer } from "@/app/game/buzzer/buzzer-container";
import { ManagementContainer } from "@/app/game/management/management-container";
import { BuzzerType } from "@/app/game/types/game-types";

type ActivityParams = {
  buzzerList: BuzzerType[];
};

export function ActivityContainer({ buzzerList }: ActivityParams) {
  const [isLocked, setLocked] = useState(false);
  const [buzzerArr, setBuzzerArr] = useState<BuzzerType[]>(buzzerList);

  function handleResetClick() {
    buzzerArr.map((buzzer) => (buzzer.isPressed = false));
    setBuzzerArr([...buzzerArr]);
  }

  function handleLockClick() {
    let newLocked = !isLocked;

    setLocked(newLocked);
    buzzerArr.map((buzzer) => (buzzer.isLocked = newLocked));
    setBuzzerArr(buzzerArr);
  }

  return (
    <div>
      <BuzzerContainer
        buzzers={buzzerArr}
        isAllLocked={isLocked}
        onLockClick={handleLockClick}
        onResetClick={handleResetClick}
      />
      <Divider className={"mt-5 mb-5"} />
      <ManagementContainer buzzers={buzzerArr} />
    </div>
  );
}
