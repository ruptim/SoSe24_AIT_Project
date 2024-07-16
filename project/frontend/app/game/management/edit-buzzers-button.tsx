"use client";
import { Button } from "@nextui-org/button";
import {
  Modal,
  ModalBody,
  ModalContent,
  ModalFooter,
  ModalHeader,
  useDisclosure,
} from "@nextui-org/modal";
import React from "react";
import {
  Table,
  TableBody,
  TableCell,
  TableColumn,
  TableHeader,
  TableRow,
  Tooltip,
} from "@nextui-org/react";

import { DeleteIcon } from "@/components/icons";
import { BuzzerType } from "@/app/game/types/game-types";

type EditBuzzersButtonParams = {
  buzzers: BuzzerType[];
  onDeleteClick: (buzzer: BuzzerType) => void;
};

export function EditBuzzersButton({
  buzzers,
  onDeleteClick,
}: EditBuzzersButtonParams) {
  const { isOpen, onOpen, onOpenChange } = useDisclosure();

  const columns = [
    {
      key: "buzzerName",
      label: "BUZZER NAME",
    },
    {
      key: "remove",
      label: "REMOVE",
    },
  ];

  const renderCell = React.useCallback(
    (item: BuzzerType, columnKey: string | number) => {
      // @ts-ignore
      const cellValue = item[columnKey];

      switch (columnKey) {
        case "remove":
          return (
            <div
              className="relative flex items-center gap-2"
              onClick={(event) => onDeleteClick(item)}
            >
              <Tooltip color="danger" content="Delete Buzzer">
                <span className="text-lg text-danger cursor-pointer active:opacity-50">
                  <DeleteIcon />
                </span>
              </Tooltip>
            </div>
          );
        default:
          return cellValue;
      }
    },
    [],
  );

  return (
    <div>
      <Button color={"default"} onPress={onOpen}>
        Edit Buzzers
      </Button>
      <Modal isOpen={isOpen} onOpenChange={onOpenChange}>
        <ModalContent>
          {(onClose) => (
            <>
              <ModalHeader className="flex flex-col gap-1">
                Edit buzzers
              </ModalHeader>
              <ModalBody>
                <Table>
                  <TableHeader columns={columns}>
                    {(column) => (
                      <TableColumn
                        key={column.key}
                        align={column.key === "remove" ? "center" : "start"}
                      >
                        {column.label}
                      </TableColumn>
                    )}
                  </TableHeader>
                  <TableBody items={buzzers}>
                    {(buzzer) => (
                      <TableRow key={buzzer.buzzerId}>
                        {(columnKey) => (
                          <TableCell>{renderCell(buzzer, columnKey)}</TableCell>
                        )}
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
  );
}
