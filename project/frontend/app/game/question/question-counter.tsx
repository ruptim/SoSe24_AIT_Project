import { Chip } from "@nextui-org/chip";

type QuestionCounterParams = {
  currentCount: number;
  maxCount: number;
};

export function QuestionCounter({
  currentCount,
  maxCount,
}: QuestionCounterParams) {
  return (
    <Chip>
      {currentCount} / {maxCount}
    </Chip>
  );
}
