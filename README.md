# WordleSolver

A C++ command-line application to help solve Wordle puzzles by suggesting optimal guesses based on previous feedback.

## Requirements

- A C++17-compatible compiler (e.g., `g++`).

## Building

Compile the source file directly:

```bash
 g++ -std=c++17 -o wordlesolver main.cpp
```

## Usage

Run WordleSolver with a dictionary file (`dict/words.txt`) containing one word per line:

```bash
 ./wordlesolver dict/words.txt
```

Once started, use these commands:

- `get`  
  Suggests the next best guess from remaining possibilities.

- `q <word> <result>`  
  Submit feedback for a guess:
  - `<word>`: your guessed word (must match the solver’s word length).
  - `<result>`: a contiguous string of digits (`0`, `1`, `2`) equal to the word length, representing each letter’s feedback:
    - `0`: letter not in the solution
    - `1`: letter in the solution but wrong position
    - `2`: letter in the correct position

  Example:
  ```bash
  q crane 01021
  ```

- `list`  
  Displays all remaining possible solution words.

- `erase <word>`  
  Removes a word from both suggestion and solution lists.

- `exit`  
  Quit the application.

## Feedback Format Details

When using `q`, ensure the `<result>` string has exactly one digit per letter of `<word>`, with no spaces or separators. Each digit corresponds to the letter at the same position in your guess.
