"use client";

import { Card, CardBody, CardHeader } from "@nextui-org/card";
import { Chip } from "@nextui-org/chip";

type BuzzerParams = {
  buzzerRank: number | null;
  buzzerName: string;
  isPressed: boolean;
  isLocked: boolean;
  delay: number | null;
  delayLocal: number | null;
};

export function Buzzer({
  buzzerRank,
  buzzerName,
  isLocked,
  isPressed,
  delay,
  delayLocal
}: BuzzerParams) {
  return (
    <Card className="h-full">
      <CardHeader className="pb-0 pt-2 px-4 flex-col items-start">
        <p className="text-tiny uppercase font-bold text-ellipsis overflow-hidden h-8 text-left">
          {buzzerName}
        </p>
        <small className="text-default-500">
          {buzzerRank ? `#${buzzerRank}` : ""}
        </small>
        <small className="text-default-500">
          {delayLocal?.toFixed(3)} {delayLocal != null ? "s" : ""}
        </small>
        <small className="text-default-500">
          {delay ? "Total: " : ""} {delay?.toFixed(3)} {delay ? "s" : ""}
        </small>
      </CardHeader>
      <CardBody>
        <div className={"flex flex-row justify-center h-full"}>
          <Chip
            className={`self-end min-w-full text-center ${isLocked ? "border-3 border-yellow-500" : ""}`}
            color={isPressed ? "success" : "default"}
          >
            {isPressed ? "" : "not"} pressed
          </Chip>
        </div>
      </CardBody>
    </Card>
  );
}
