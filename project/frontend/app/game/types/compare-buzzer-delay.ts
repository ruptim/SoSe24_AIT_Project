import { BuzzerType } from "@/app/game/types/game-types";

export function compareBuzzerDelay(
  buzzer1: BuzzerType,
  buzzer2: BuzzerType,
): number {
  const delay1 = buzzer1.delay ?? Number.POSITIVE_INFINITY; // Default to positive infinity if delay is undefined
  const delay2 = buzzer2.delay ?? Number.POSITIVE_INFINITY;

  if (delay1 < delay2) {
    return -1; // buzzer1 has smaller delay
  } else if (delay1 > delay2) {
    return 1; // buzzer1 has larger delay
  } else {
    return 0; // Both buzzers have equal delay
  }
}
