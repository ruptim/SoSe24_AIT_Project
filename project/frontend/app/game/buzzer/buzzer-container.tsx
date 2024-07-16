"use client";

import { Buzzer } from "@/app/game/buzzer/buzzer";
import { BuzzerResetButton } from "@/app/game/buzzer/buzzer-reset-button";
import { BuzzerLockButton } from "@/app/game/buzzer/buzzer-lock-button";
import { BuzzerType } from "@/app/game/types/game-types";
import { compareBuzzerDelay } from "@/app/game/types/compare-buzzer-delay";

type BuzzerContainerParams = {
  buzzers: BuzzerType[];
  isAllLocked: boolean;
  onResetClick: () => void;
  onLockClick: () => void;
};

export function BuzzerContainer({
  buzzers,
  onResetClick,
  onLockClick,
  isAllLocked,
}: BuzzerContainerParams) {
  return (
    <div>
      <div className={"flex flex-row justify-center gap-5 flex-wrap"}>
        {buzzers
          .sort((a, b) => compareBuzzerDelay(a, b))
          .map((buzzer, index) => (
            <div key={buzzer.buzzerId} className={"box-border h-34 w-40"}>
              <Buzzer
                buzzerName={buzzer.buzzerName}
                buzzerRank={index + 1}
                delay={buzzer.delay}
                isLocked={buzzer.isLocked}
                isPressed={buzzer.isPressed}
              />
            </div>
          ))}
      </div>
      <div className={"flex flex-row justify-center gap-5 mt-5"}>
        <BuzzerResetButton onResetClick={onResetClick} />
      </div>
    </div>
  );
}
