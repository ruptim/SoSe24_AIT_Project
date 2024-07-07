'use client'
import {Button} from "@nextui-org/button";
import {Modal, ModalBody, ModalContent, ModalFooter, ModalHeader, useDisclosure} from "@nextui-org/modal";
import React from "react";
import {Spinner} from "@nextui-org/spinner";
import {Buzzer} from "@/app/game/buzzer/buzzer";
import {BuzzerType} from "@/app/game/types/game-types";
import {Divider} from "@nextui-org/divider";

type ConnectBuzzerButtonParams = {
    isPairing: boolean,
    buzzersShown: BuzzerType[],
    onOpenClicked: () => void,
    onModalClosed: () => void
}

export function ConnectBuzzerButton({
                                        isPairing,
                                        buzzersShown,
                                        onOpenClicked,
                                        onModalClosed
                                    }: ConnectBuzzerButtonParams) {
    const {isOpen, onOpen, onOpenChange} = useDisclosure();

    return (
        <div>
            <Button color={"default"} onPress={onOpen} onClick={onOpenClicked}>Connect Buzzer</Button>
            <Modal isOpen={isOpen} onOpenChange={onOpenChange} onClose={onModalClosed}>
                <ModalContent>
                    {(onClose) => (
                        <>
                            <ModalHeader className="flex flex-col gap-1">Connect a new buzzer</ModalHeader>
                            <ModalBody>
                                <p>
                                    To connect a new buzzer, please press and hold it down until the light begins to
                                    flash.
                                </p>
                                {isPairing ?
                                    (<div className={"flex justify-center mt-5"}>
                                        <Spinner size="lg"/>
                                    </div>) : ''
                                }
                                {buzzersShown.length ?
                                    (<>
                                        <Divider className={"mt-2 mb-2"}></Divider>
                                        <p>
                                            New Buzzer{buzzersShown.length > 1 ? 's' : ''} connected:
                                        </p>
                                        <div className={"box-border w-full flex justify-center gap-2 flex-wrap"}>
                                            {buzzersShown.map(buzzer => (
                                                <div className={"box-border h-32 w-32"} key={buzzer.buzzerId}>
                                                   <Buzzer buzzerRank={null} buzzerName={buzzer.buzzerName} isPressed={buzzer.isPressed} isLocked={buzzer.isLocked} delay={null}></Buzzer>
                                                </div>

                                            ))}
                                        </div>
                                    </>) : ''}
                            </ModalBody>
                            <ModalFooter>
                                <Button color="danger" variant="light" onPress={onClose}>
                                    Close
                                </Button>
                            </ModalFooter>
                        </>
                    )}
                </ModalContent>
            </Modal>
        </div>
    )
}