//=============================================================================
// Command.cpp
//=============================================================================
// PURPOSE:
//   Implementation of the undo/redo command classes and the CommandManager
//   which manages command history for undo/redo operations.
//
// NOTES:
//   - Concrete commands (Add/Delete/Change...) encapsulate state changes so
//     they can be executed, undone, and redone.
//   - CommandManager keeps bounded history and prints simple debug output.
//=============================================================================

#include "Command.h"
#include <iostream>

// ============== AddCharacterCommand ==============

AddCharacterCommand::AddCharacterCommand(std::vector<std::unique_ptr<Character>>& chars,
                                       std::unique_ptr<Character> ch)
    : characters(chars), character(std::move(ch)), isExecuted(false) {}

void AddCharacterCommand::execute() {
    characters.push_back(std::move(character));
    isExecuted = true;
}

void AddCharacterCommand::undo() {
    if (isExecuted && !characters.empty()) {
        character = std::move(characters.back());
        characters.pop_back();
        isExecuted = false;
    }
}

std::string AddCharacterCommand::getName() const {
    return "Add Character";
}

// ============== AddBubbleCommand ==============

AddBubbleCommand::AddBubbleCommand(std::vector<std::unique_ptr<SpeechBubble>>& bubs,
                                 std::unique_ptr<SpeechBubble> b)
    : bubbles(bubs), bubble(std::move(b)), isExecuted(false) {}

void AddBubbleCommand::execute() {
    bubbles.push_back(std::move(bubble));
    isExecuted = true;
}

void AddBubbleCommand::undo() {
    if (isExecuted && !bubbles.empty()) {
        bubble = std::move(bubbles.back());
        bubbles.pop_back();
        isExecuted = false;
    }
}

std::string AddBubbleCommand::getName() const {
    return "Add Bubble";
}

// ============== AddStrokeCommand ==============

AddStrokeCommand::AddStrokeCommand(std::vector<std::unique_ptr<BrushStroke>>& stks,
                                 std::unique_ptr<BrushStroke> s)
    : strokes(stks), stroke(std::move(s)), isExecuted(false) {}

void AddStrokeCommand::execute() {
    strokes.push_back(std::move(stroke));
    isExecuted = true;
}

void AddStrokeCommand::undo() {
    if (isExecuted && !strokes.empty()) {
        stroke = std::move(strokes.back());
        strokes.pop_back();
        isExecuted = false;
    }
}

std::string AddStrokeCommand::getName() const {
    return "Add Stroke";
}

// ============== DeleteCharacterCommand ==============

DeleteCharacterCommand::DeleteCharacterCommand(std::vector<std::unique_ptr<Character>>& chars, int idx)
    : characters(chars), index(idx), isExecuted(false) {}

void DeleteCharacterCommand::execute() {
    if (index >= 0 && index < static_cast<int>(characters.size())) {
        character = std::move(characters[index]);
        characters.erase(characters.begin() + index);
        isExecuted = true;
    }
}

void DeleteCharacterCommand::undo() {
    if (isExecuted && character) {
        characters.insert(characters.begin() + index, std::move(character));
        isExecuted = false;
    }
}

std::string DeleteCharacterCommand::getName() const {
    return "Delete Character";
}

// ============== DeleteBubbleCommand ==============

DeleteBubbleCommand::DeleteBubbleCommand(std::vector<std::unique_ptr<SpeechBubble>>& bubs, int idx)
    : bubbles(bubs), index(idx), isExecuted(false) {}

void DeleteBubbleCommand::execute() {
    if (index >= 0 && index < static_cast<int>(bubbles.size())) {
        bubble = std::move(bubbles[index]);
        bubbles.erase(bubbles.begin() + index);
        isExecuted = true;
    }
}

void DeleteBubbleCommand::undo() {
    if (isExecuted && bubble) {
        bubbles.insert(bubbles.begin() + index, std::move(bubble));
        isExecuted = false;
    }
}

std::string DeleteBubbleCommand::getName() const {
    return "Delete Bubble";
}

// ============== ChangeBubbleFontSizeCommand ==============

ChangeBubbleFontSizeCommand::ChangeBubbleFontSizeCommand(SpeechBubble* b, int oldSize, int newSize)
    : bubble(b), previousFontSize(oldSize), newFontSize(newSize), isExecuted(false) {}

void ChangeBubbleFontSizeCommand::execute() {
    if (bubble) {
        bubble->setFontSize(newFontSize);
        isExecuted = true;
    }
}

void ChangeBubbleFontSizeCommand::undo() {
    if (isExecuted && bubble) {
        bubble->setFontSize(previousFontSize);
        isExecuted = false;
    }
}

std::string ChangeBubbleFontSizeCommand::getName() const {
    return "Change Font Size";
}

void CommandManager::executeCommand(std::unique_ptr<Command> cmd) {
    cmd->execute();
    undoStack.push_back(std::move(cmd));
    
    // Clear redo stack when new command is executed
    redoStack.clear();
    
    // Limit history size
    if (undoStack.size() > maxHistorySize) {
        undoStack.erase(undoStack.begin());
    }
    
    std::cout << "[Undo] Command executed: " << undoStack.back()->getName() << "\n";
}

bool CommandManager::canUndo() const {
    return !undoStack.empty();
}

bool CommandManager::canRedo() const {
    return !redoStack.empty();
}

void CommandManager::undo() {
    if (canUndo()) {
        auto cmd = std::move(undoStack.back());
        undoStack.pop_back();
        cmd->undo();
        redoStack.push_back(std::move(cmd));
        std::cout << "[Undo] Undid: " << redoStack.back()->getName() << "\n";
    }
}

void CommandManager::redo() {
    if (canRedo()) {
        auto cmd = std::move(redoStack.back());
        redoStack.pop_back();
        cmd->execute();
        undoStack.push_back(std::move(cmd));
        std::cout << "[Redo] Redid: " << undoStack.back()->getName() << "\n";
    }
}

void CommandManager::clear() {
    undoStack.clear();
    redoStack.clear();
}

size_t CommandManager::getUndoCount() const {
    return undoStack.size();
}

size_t CommandManager::getRedoCount() const {
    return redoStack.size();
}
