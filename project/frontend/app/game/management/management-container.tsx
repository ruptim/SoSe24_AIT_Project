import { useState } from "react";

import { ConnectBuzzerButton } from "@/app/game/management/connect-buzzer-button";
import { EditBuzzersButton } from "@/app/game/management/edit-buzzers-button";
import { BuzzerType } from "@/app/game/types/game-types";

type ManagementContainerParams = {
  buzzers: BuzzerType[];
};

export function ManagementContainer({ buzzers }: ManagementContainerParams) {
  const [isPairing, setPairing] = useState(false);
  const [buzzerArr, setBuzzerArr] = useState<BuzzerType[]>(buzzers);
  const [newBuzzerArr, setNewBuzzerArr] = useState<BuzzerType[]>([]);

  function connectModalOpened() {
    setPairing(true);
    setNewBuzzerArr([]);
    getNewBuzzers();
  }

  function connectModalClosed() {
    setPairing(false);
  }

  function getNewBuzzers() {
    setTimeout(() => {
      setNewBuzzerArr([
        {
          buzzerId: 2,
          buzzerName: "New Buzzer",
          isPressed: false,
          isLocked: false,
          delay: null,
        },
        {
          buzzerId: 3,
          buzzerName: "New Second Buzzer",
          isPressed: false,
          isLocked: false,
          delay: null,
        },
      ]);
      setPairing(false);
    }, 1000);
  }

  function deleteBuzzer(buzzer: BuzzerType) {
    console.log(buzzer);
  }

  return (
    <div className={"flex flex-row gap-5 justify-start"}>
      <ConnectBuzzerButton
        buzzersShown={newBuzzerArr}
        isPairing={isPairing}
        onModalClosed={connectModalClosed}
        onOpenClicked={connectModalOpened}
      />
      <EditBuzzersButton buzzers={buzzerArr} onDeleteClick={deleteBuzzer} />
    </div>
  );
}
