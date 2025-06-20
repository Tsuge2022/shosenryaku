#pragma once
#include <Siv3D.hpp> // For Key, Mouse, Point types

// This class can be a simple wrapper around Siv3D's input functions,
// or it can be more complex to handle input mapping, buffering, etc.
// For now, a simple wrapper.

class InputManager
{
public:
    InputManager();  // Constructor
    ~InputManager(); // Destructor

    // Mouse Input
    bool IsMouseButtonClicked(MouseButtons button = MouseL) const; // Check for a click (down event)
    bool IsMouseButtonPressed(MouseButtons button = MouseL) const; // Check if button is currently held down
    bool IsMouseButtonReleased(MouseButtons button = MouseL) const; // Check for a release (up event)
    Point GetMousePosition() const; // Get current mouse cursor position
    Vec2 GetMouseDelta() const;    // Get mouse movement since last frame
    double GetMouseWheel() const;  // Get mouse wheel scroll amount

    // Keyboard Input
    bool IsKeyPressed(const Key& key) const;      // Check if key is currently held down
    bool IsKeyClicked(const Key& key) const;      // Check for a key down event
    bool IsKeyReleased(const Key& key) const;     // Check for a key up event
    String GetTextInput() const;                  // Get text input this frame

    // Gamepad/Controller Input (Example - Siv3D has more detailed gamepad support)
    // bool IsGamepadButtonPressed(int gamepadIndex, GamepadButton button) const;
    // Vec2 GetGamepadAxis(int gamepadIndex, GamepadAxis axis) const;

    void Update(); // Call at the beginning of each frame to update internal states if needed (e.g., for click detection)

private:
    // Could store previous input states here to implement custom click/release detection
    // if Siv3D's built-in `down()`, `pressed()`, `up()` are not sufficient or if more control is needed.
    // For example:
    // MouseState prevMouseState;
    // KeyboardState prevKeyboardState;
};
