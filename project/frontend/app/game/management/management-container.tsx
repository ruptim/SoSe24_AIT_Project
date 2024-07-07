import { useState } from "react";

import { ConnectBuzzerButton } from "@/app/game/management/connect-buzzer-button";
import { EditBuzzersButton } from "@/app/game/management/edit-buzzers-button";
import { BuzzerType } from "@/app/game/types/game-types";
import {socket} from "@/app/socket";
import {backendConfig} from "@/config/backend-config";

type ManagementContainerParams = {
  buzzers: BuzzerType[];
  newBuzzers: BuzzerType[];
  onBuzzerDelete: (buzzer: BuzzerType) => void
};

export function ManagementContainer({ buzzers, newBuzzers, onBuzzerDelete }: ManagementContainerParams) {
  const [isPairing, setPairing] = useState(false);
  const [newBuzzerArr, setNewBuzzerArr] = useState<BuzzerType[]>(newBuzzers);

  function connectModalOpened() {
    setPairing(true);
    setNewBuzzerArr([]);
    // getNewBuzzers();
    socket.emit(backendConfig.events.pairing);
  }

  function connectModalClosed() {
    setPairing(false);
  }

  // function getNewBuzzers() {
  //   setTimeout(() => {
  //     setNewBuzzerArr([
  //       {
  //         buzzerId: 2,
  //         buzzerName: "New Buzzer",
  //         isPressed: false,
  //         isLocked: false,
  //         delay: null,
  //       },
  //       {
  //         buzzerId: 3,
  //         buzzerName: "New Second Buzzer",
  //         isPressed: false,
  //         isLocked: false,
  //         delay: null,
  //       },
  //     ]);
  //     setPairing(false);
  //   }, 1000);
  // }

  function deleteBuzzer(buzzer: BuzzerType) {
    console.log('DELETE: ' + buzzer);
    onBuzzerDelete(buzzer);
  }

  return (
    <div className={"flex flex-row gap-5 justify-start"}>
      <ConnectBuzzerButton
        buzzersShown={newBuzzerArr}
        isPairing={isPairing}
        onModalClosed={connectModalClosed}
        onOpenClicked={connectModalOpened}
      />
      <EditBuzzersButton buzzers={buzzers} onDeleteClick={deleteBuzzer} />
    </div>
  );
}
