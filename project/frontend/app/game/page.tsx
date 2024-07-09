import {QuestionContainer} from "@/app/game/question/question-container";
import {Divider} from "@nextui-org/divider";
import {ActivityContainer} from "@/app/game/activity/activity-container";
import {BuzzerType, QuestionType} from "@/app/game/types/game-types";

export default function GamePage() {

    let buzzerList: BuzzerType[] = [
        {
            buzzerId: 0,
            buzzerName: 'First Buzzer',
            isPressed: true,
            isLocked: false,
            delay: 2.56
        },
        {
            buzzerId: 1,
            buzzerName: 'Second Buzzer that has a long name',
            isPressed: false,
            isLocked: false,
            delay: null
        }
    ]

    let questions: QuestionType[] = [
        {
            question: "Was ist das Internet der Dinge (IoT)?",
            answer: "Das IoT ist ein Netzwerk physischer Objekte oder Personen, die mit Software, Elektronik, Netzwerken und Sensoren ausgestattet sind. Diese Objekte sammeln und tauschen Daten aus, um sie intelligenter und effizienter zu machen."
        },
        {
            question: "Was sind die grundlegenden Komponenten eines IoT-Systems?",
            answer: "Die vier grundlegenden Komponenten sind Sensoren/Devices, Konnektivität, Datenverarbeitung und Benutzeroberfläche."
        },
        {
            question: "Was ist der Unterschied zwischen IoT und IIoT?",
            answer: "IoT ist auf kundenorientierte Anwendungen ausgerichtet, während IIoT industrielle Anwendungen unterstützt. IIoT verwendet sowohl drahtgebundene als auch drahtlose Kommunikation und bietet hochwertige Daten."
        },
        {
            question: "Was ist Raspberry Pi?",
            answer: "Raspberry Pi ist ein Computer, der ähnlich wie ein herkömmlicher Computer funktioniert. Er verfügt über Funktionen wie WLAN, GPIO-Pins und Bluetooth zur Kommunikation mit externen Geräten."
        },
        {
            question: "Wie läuft Raspberry Pi im Headless-Modus?",
            answer: "Raspberry Pi kann im Headless-Modus über SSH betrieben werden. Das neueste Betriebssystem verfügt über einen integrierten VNC-Server für die Fernsteuerung."
        }
    ]


    return (
    <div className="w-full max-w-screen-md min-w-full">
        <QuestionContainer questions={questions}></QuestionContainer>
        <Divider className={"mt-5 mb-5"}></Divider>
        <ActivityContainer buzzerList={buzzerList}></ActivityContainer>
    </div>
  );
}
