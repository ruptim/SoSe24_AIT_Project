'use client'
import {Button} from "@nextui-org/button";
import {Modal, ModalBody, ModalContent, ModalFooter, ModalHeader, useDisclosure} from "@nextui-org/modal";
import React from "react";
import {getKeyValue, Table, TableBody, TableCell, TableColumn, TableHeader, TableRow, Tooltip} from "@nextui-org/react";
import { DeleteIcon } from "@/components/icons";

export function EditBuzzersButton(){
    const {isOpen, onOpen, onOpenChange} = useDisclosure();

    let rows = [
        {
            key: "0",
            id: "0",
            name: "Buzzer Robin",
        },
        {
            key: "1",
            id: "1",
            name: "Timons Buzzer",
        }
    ]

    const columns = [
        {
            key: "id",
            label: "ID",
        },
        {
            key: "name",
            label: "NAME",
        },
        {
            key: "remove",
            label: "REMOVE",
        },
    ];

    const renderCell = React.useCallback((item: {key: string, id: string, name: string}, columnKey: string | number) => {
        // @ts-ignore
        const cellValue = item[columnKey];

        switch (columnKey) {
            case "remove":
                return (
                    <div className="relative flex items-center gap-2">
                        <Tooltip color="danger" content="Delete user">
                          <span className="text-lg text-danger cursor-pointer active:opacity-50">
                            <DeleteIcon />
                          </span>
                        </Tooltip>
                    </div>
                );
            default:
                return cellValue;
        }
    }, []);

    return (
        <div>
            <Button color={"default"} onPress={onOpen}>Edit Buzzers</Button>
            <Modal isOpen={isOpen} onOpenChange={onOpenChange}>
                <ModalContent>
                    {(onClose) => (
                        <>
                            <ModalHeader className="flex flex-col gap-1">Edit buzzers</ModalHeader>
                            <ModalBody>
                                <Table>
                                    <TableHeader columns={columns}>
                                        {(column) => (
                                            <TableColumn key={column.key} align={column.key === "remove" ? "center" : "start"}>
                                                {column.label}
                                            </TableColumn>
                                        )}
                                    </TableHeader>
                                    <TableBody items={rows}>
                                        {(item) => (
                                            <TableRow key={item.key}>
                                                {(columnKey) => <TableCell>{renderCell(item, columnKey)}</TableCell>}
                                            </TableRow>
                                        )}
                                    </TableBody>
                                </Table>
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