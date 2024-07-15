export type BuzzerType = {
  buzzerId: number;
  buzzerName: string;
  isPressed: boolean;
  isLocked: boolean;
  delay: number | null;
};

export type QuestionType = {
  question: string;
  answer: string;
};
