'use client'

import {Divider} from "@nextui-org/divider";

import {QuestionContainer} from "@/app/game/question/question-container";
import {ActivityContainer} from "@/app/game/activity/activity-container";
import {BuzzerType, QuestionType} from "@/app/game/types/game-types";
import {useEffect, useState} from "react";
import { socket} from "@/app/socket";
import {backendConfig} from "@/config/backend-config";
import {ConnectionContainer} from "@/app/game/connection/connection-container";
import {Button} from "@nextui-org/button";
import {Socket} from "socket.io-client";
import {DefaultEventsMap} from "@socket.io/component-emitter";
import questions from '../../config/questions.json'

export default function GamePage() {
  const [appSocket, setSocket] = useState< Socket<DefaultEventsMap, DefaultEventsMap>>(socket)
  const [isConnected, setIsConnected] = useState(appSocket.connected);
  const [buzzers, setBuzzers] = useState<BuzzerType[]>([]);

  useEffect(() => {

    function onConnect() {
      setIsConnected(true);
      console.log('Connected');
    }

    function onDisconnect() {
      setIsConnected(false);
      console.log('Disconnected');
    }

    function onBuzzerUpdate(receivedBuzzersString: string) {
      try{
        let receivedBuzzers: BuzzerType[] = JSON.parse(receivedBuzzersString);
        setBuzzers(receivedBuzzers);
      } catch (e){
        console.error(e)
      }
    }

    socket.on(backendConfig.events.connect, onConnect);
    socket.on(backendConfig.events.disconnect, onDisconnect);
    socket.on(backendConfig.events.buzzers, onBuzzerUpdate);

    socket.connect();
    setIsConnected(appSocket.connected);

    return () => {
      socket.off(backendConfig.events.connect, onConnect);
      socket.off(backendConfig.events.disconnect, onDisconnect);
      socket.off(backendConfig.events.buzzers, onBuzzerUpdate);
      socket.close()
    };
  }, []);

  return (
    <div className="w-full max-w-screen-md min-w-full flex flex-col">
      <QuestionContainer questions={questions.questions}/>
      <Divider className={"mt-5 mb-5"}/>
      <ActivityContainer buzzers={buzzers}/>
      <div className="self-end">
        <ConnectionContainer isConnected={isConnected}></ConnectionContainer>
      </div>
    </div>
  );
}
