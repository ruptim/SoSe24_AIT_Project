'use client'
import {Button} from "@nextui-org/button";
import {Modal, ModalBody, ModalContent, ModalFooter, ModalHeader, useDisclosure} from "@nextui-org/modal";
import React from "react";
import {Spinner} from "@nextui-org/spinner";
import {Buzzer} from "@/app/game/buzzer/buzzer";

export function ConnectBuzzerButton(){
    const {isOpen, onOpen, onOpenChange} = useDisclosure();

    return (
        <div>
            <Button color={"default"} onPress={onOpen}>Connect Buzzer</Button>
            <Modal isOpen={isOpen} onOpenChange={onOpenChange}>
                <ModalContent>
                    {(onClose) => (
                        <>
                            <ModalHeader className="flex flex-col gap-1">Connect a new buzzer</ModalHeader>
                            <ModalBody>
                                <p>
                                    To connect a new buzzer, please double press and hold it now.
                                </p>
                                <div className={"flex justify-center mt-5"}>
                                    <Spinner size="lg" />
                                </div>
                                <p>
                                    New Buzzer connected:
                                </p>
                                <div className={"box-border w-1/3 flex justify-center"}>
                                    <Buzzer buzzerId={4} buzzerName={"New Buzzer"} isPressed={false}></Buzzer>
                                </div>
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