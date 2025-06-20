#include "InputManager.h"

InputManager::InputManager()
{
    // Constructor implementation
    // Initialize any previous input states if used for custom click detection
}

InputManager::~InputManager()
{
    // Destructor implementation
}

void InputManager::Update()
{
    // Call at the beginning of each frame.
    // If you were implementing custom click/release detection that Siv3D doesn't provide
    // out-of-the-box (e.g., for more complex gesture systems or specific timing needs),
    // you would update your `prevMouseState` and `prevKeyboardState` here.
    // For most common use cases, Siv3D's `Key::down()`, `Key::pressed()`, `Key::up()`
    // and `MouseL.down()`, etc., are sufficient and don't require manual state tracking here.
}

// Mouse Input
bool InputManager::IsMouseButtonClicked(MouseButtons button) const
{
    return button.down();
}

bool InputManager::IsMouseButtonPressed(MouseButtons button) const
{
    return button.pressed();
}

bool InputManager::IsMouseButtonReleased(MouseButtons button) const
{
    return button.up();
}

Point InputManager::GetMousePosition() const
{
    return Cursor::Pos();
}

Vec2 InputManager::GetMouseDelta() const
{
    return Cursor::Delta();
}

double InputManager::GetMouseWheel() const
{
    return Mouse::Wheel();
}

// Keyboard Input
bool InputManager::IsKeyPressed(const Key& key) const
{
    return key.pressed();
}

bool InputManager::IsKeyClicked(const Key& key) const
{
    return key.down();
}

bool InputManager::IsKeyReleased(const Key& key) const
{
    return key.up();
}

String InputManager::GetTextInput() const
{
    // This gets all text input events this frame concatenated.
    // For more detailed input (e.g. individual characters with timestamps),
    // you might need to process TextInput::GetEvents()
    String text;
    for (const auto& ch : TextInput::GetChars())
    {
        text += ch;
    }
    return text;
}

// Gamepad/Controller Input (Example stubs)
// bool InputManager::IsGamepadButtonPressed(int gamepadIndex, GamepadButton button) const
// {
//     if (auto gamepad = Gamepad(gamepadIndex))
//     {
//         // return gamepad.button(button).down(); // Or .pressed() / .up()
//     }
//     return false;
// }

// Vec2 InputManager::GetGamepadAxis(int gamepadIndex, GamepadAxis axis) const
// {
//     if (auto gamepad = Gamepad(gamepadIndex))
//     {
//         // return gamepad.axes[axis]; // Example, check Siv3D docs for correct axis access
//     }
//     return Vec2::Zero();
// }
