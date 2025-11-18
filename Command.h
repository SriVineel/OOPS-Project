//=============================================================================
// Command.h
//=============================================================================
// PURPOSE:
//   Implements the Command pattern for undo/redo functionality.
//   Each user action (add character, add bubble, add stroke, delete) is a command.
//
// COMMAND PATTERN BENEFITS:
//   - Encapsulates actions as objects
//   - Supports undo/redo through execute() and undo() methods
//   - Can be stored in history stacks
//   - Enables command sequencing and macros
//
// AVAILABLE COMMANDS:
//   - AddCharacterCommand: Adds a character to scene
//   - AddBubbleCommand: Adds a speech bubble to scene
//   - AddStrokeCommand: Adds a brush stroke to scene
//   - DeleteObjectCommand: Removes an object from scene
//
// WHERE TO MODIFY:
//   - Add new commands: Create new class inheriting from Command
//   - Add command parameters: Extend constructor with needed data
//   - Implement complex undo: Store more state in command object
//   - Add command merging: Implement merge() for consecutive similar commands
//=============================================================================

#pragma once

#include <memory>
#include <vector>
#include <string>
#include "Character.h"
#include "SpeechBubble.h"
#include "BrushStroke.h"

//-----------------------------------------------------------------------------
// BASE COMMAND INTERFACE
//-----------------------------------------------------------------------------

class Command {
public:
    virtual ~Command() = default;

    // Execute the command (apply the action)
    virtual void execute() = 0;

    // Undo the command (reverse the action)
    virtual void undo() = 0;

    // Get human-readable command name for UI display
    virtual std::string getName() const = 0;
};

//-----------------------------------------------------------------------------
// ADD CHARACTER COMMAND
//-----------------------------------------------------------------------------

class AddCharacterCommand : public Command {
private:
    std::vector<std::unique_ptr<Character>>& characters;  // Reference to scene character list
    std::unique_ptr<Character> character;                 // The character to add/remove
    bool isExecuted;                                      // Track if currently in scene

public:
    // Constructor: Takes reference to scene list and character to add
    AddCharacterCommand(std::vector<std::unique_ptr<Character>>& chars,
                        std::unique_ptr<Character> ch);

    void execute() override;   // Adds character to scene
    void undo() override;      // Removes character from scene
    std::string getName() const override;
};

//-----------------------------------------------------------------------------
// ADD BUBBLE COMMAND
//-----------------------------------------------------------------------------

class AddBubbleCommand : public Command {
private:
    std::vector<std::unique_ptr<SpeechBubble>>& bubbles;  // Reference to scene bubble list
    std::unique_ptr<SpeechBubble> bubble;                 // The bubble to add/remove
    bool isExecuted;

public:
    AddBubbleCommand(std::vector<std::unique_ptr<SpeechBubble>>& bubs,
                     std::unique_ptr<SpeechBubble> b);

    void execute() override;
    void undo() override;
    std::string getName() const override;
};

//-----------------------------------------------------------------------------
// ADD STROKE COMMAND
//-----------------------------------------------------------------------------

class AddStrokeCommand : public Command {
private:
    std::vector<std::unique_ptr<BrushStroke>>& strokes;  // Reference to scene stroke list
    std::unique_ptr<BrushStroke> stroke;                 // The stroke to add/remove
    bool isExecuted;

public:
    AddStrokeCommand(std::vector<std::unique_ptr<BrushStroke>>& strks,
                     std::unique_ptr<BrushStroke> s);

    void execute() override;
    void undo() override;
    std::string getName() const override;
};

//-----------------------------------------------------------------------------
// DELETE CHARACTER COMMAND
//-----------------------------------------------------------------------------

class DeleteCharacterCommand : public Command {
private:
    std::vector<std::unique_ptr<Character>>& characters;
    std::unique_ptr<Character> character;
    int index;
    bool isExecuted;

public:
    DeleteCharacterCommand(std::vector<std::unique_ptr<Character>>& chars, int idx);

    void execute() override;
    void undo() override;
    std::string getName() const override;
};

//-----------------------------------------------------------------------------
// DELETE BUBBLE COMMAND
//-----------------------------------------------------------------------------

class DeleteBubbleCommand : public Command {
private:
    std::vector<std::unique_ptr<SpeechBubble>>& bubbles;
    std::unique_ptr<SpeechBubble> bubble;
    int index;
    bool isExecuted;

public:
    DeleteBubbleCommand(std::vector<std::unique_ptr<SpeechBubble>>& bubs, int idx);

    void execute() override;
    void undo() override;
    std::string getName() const override;
};

//-----------------------------------------------------------------------------
// CHANGE BUBBLE FONT SIZE COMMAND
//-----------------------------------------------------------------------------

class ChangeBubbleFontSizeCommand : public Command {
private:
    SpeechBubble* bubble;
    int previousFontSize;
    int newFontSize;
    bool isExecuted;

public:
    ChangeBubbleFontSizeCommand(SpeechBubble* b, int oldSize, int newSize);

    void execute() override;
    void undo() override;
    std::string getName() const override;
};

//-----------------------------------------------------------------------------
// COMMAND MANAGER
//-----------------------------------------------------------------------------

class CommandManager {
private:
    std::vector<std::unique_ptr<Command>> undoStack;
    std::vector<std::unique_ptr<Command>> redoStack;
    static constexpr size_t maxHistorySize = 100;

public:
    void executeCommand(std::unique_ptr<Command> cmd);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    void clear();
    size_t getUndoCount() const;
    size_t getRedoCount() const;
};