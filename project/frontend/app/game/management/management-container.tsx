import {useEffect, useState} from "react";

import { ConnectBuzzerButton } from "@/app/game/management/connect-buzzer-button";
import { EditBuzzersButton } from "@/app/game/management/edit-buzzers-button";
import { BuzzerType } from "@/app/game/types/game-types";
import {socket} from "@/app/socket";
import {backendConfig} from "@/config/backend-config";
import {UploadQuestionModal} from "@/app/game/management/upload-question-modal";

type ManagementContainerParams = {
  buzzers: BuzzerType[];
  onBuzzerDelete: (buzzer: BuzzerType) => void
};

export function ManagementContainer({ buzzers, onBuzzerDelete }: ManagementContainerParams) {
  const [isPairing, setPairing] = useState(false);

  function connectModalOpened() {
    setPairing(true);
    socket.emit(backendConfig.events.pairing, true);
  }

  function connectModalClosed() {
    setPairing(false);
    socket.emit(backendConfig.events.pairing, false);
  }

  function deleteBuzzer(buzzer: BuzzerType) {
    console.log('DELETE: ' + buzzer);
    onBuzzerDelete(buzzer);
  }

  return (
      <div className={"flex flex-row justify-between"}>
        <div className={"flex flex-row gap-5"}>
          <ConnectBuzzerButton
            buzzersShown={buzzers}
            isPairing={isPairing}
            onModalClosed={connectModalClosed}
            onOpenClicked={connectModalOpened}
          />
          <EditBuzzersButton buzzers={buzzers} onDeleteClick={deleteBuzzer} />
        </div>
        <div>
          {/*<UploadQuestionModal></UploadQuestionModal>*/}
        </div>
      </div>
  );
}
