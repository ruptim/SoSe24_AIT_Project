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

  let questions: QuestionType[] = [
    {
      question: "Was ist das Internet der Dinge (IoT)?",
      options: [
        {optionText: "Option A", isCorrect: false},
        {optionText: "Option B", isCorrect: true},
        {optionText: "Option C", isCorrect: false},
        {optionText: "Option D", isCorrect: false}
      ],
      answer:
        "Das IoT ist ein Netzwerk physischer Objekte oder Personen, die mit Software, Elektronik, Netzwerken und Sensoren ausgestattet sind. Diese Objekte sammeln und tauschen Daten aus, um sie intelligenter und effizienter zu machen.",
    },
    {
      question: "Was sind die grundlegenden Komponenten eines IoT-Systems?",
      options: [
        {optionText: "Option A", isCorrect: false},
        {optionText: "Option B", isCorrect: true},
        {optionText: "Option C", isCorrect: false},
        {optionText: "Option D", isCorrect: true}
      ],
      answer:
        "Die vier grundlegenden Komponenten sind Sensoren/Devices, Konnektivität, Datenverarbeitung und Benutzeroberfläche.",
    },
    {
      question: "Was ist der Unterschied zwischen IoT und IIoT?",
      options: [],
      answer:
        "IoT ist auf kundenorientierte Anwendungen ausgerichtet, während IIoT industrielle Anwendungen unterstützt. IIoT verwendet sowohl drahtgebundene als auch drahtlose Kommunikation und bietet hochwertige Daten.",
    },
    {
      question: "Was ist Raspberry Pi?",
      options: [],
      answer:
        "Raspberry Pi ist ein Computer, der ähnlich wie ein herkömmlicher Computer funktioniert. Er verfügt über Funktionen wie WLAN, GPIO-Pins und Bluetooth zur Kommunikation mit externen Geräten.",
    },
    {
      question: "Wie läuft Raspberry Pi im Headless-Modus?",
      options: [],
      answer:
        "Raspberry Pi kann im Headless-Modus über SSH betrieben werden. Das neueste Betriebssystem verfügt über einen integrierten VNC-Server für die Fernsteuerung.",
    },
  ];

  return (
    <div className="w-full max-w-screen-md min-w-full flex flex-col">
      <QuestionContainer questions={questions}/>
      <Divider className={"mt-5 mb-5"}/>
      <ActivityContainer buzzers={buzzers}/>
      <div className="self-end">
        <ConnectionContainer isConnected={isConnected}></ConnectionContainer>
      </div>
    </div>
  );
}
