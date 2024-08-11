import time
from strafe_tapper import check_key_state, hold_key, release_held_key
from config import get_config

def main():
    config = get_config()
    counter_tap_hold_time = config['hold_time']  # Retrieve hold time for counter-tap

    print("Listening for 'A' and 'D' key releases with SnapKey feature. Press 'N' to toggle functionality. Press Ctrl+C to quit.")

    enabled = True
    active_key = None  # Tracks the currently active key
    previous_key = None  # Tracks the previously active key

    while True:
        try:
            if check_key_state('N'):  # Toggle functionality
                enabled = not enabled
                print("Functionality enabled." if enabled else "Functionality disabled.")
                while check_key_state('N'):  # Wait until 'N' is released
                    time.sleep(0.05)

            if enabled:
                a_pressed = check_key_state('A')
                d_pressed = check_key_state('D')

                if a_pressed and d_pressed:
                    # Both 'A' and 'D' are pressed
                    print("Both 'A' and 'D' pressed. Switching to active second key.")
                    if active_key != 'D':
                        if active_key == 'A':
                            release_held_key('A')
                        hold_key('D')
                        previous_key = 'A'
                        active_key = 'D'
                elif a_pressed:
                    if active_key != 'A':
                        if active_key == 'D':
                            release_held_key('D')
                        hold_key('A')
                        active_key = 'A'
                        previous_key = 'D'
                elif d_pressed:
                    if active_key != 'D':
                        if active_key == 'A':
                            release_held_key('A')
                        hold_key('D')
                        active_key = 'D'
                        previous_key = 'A'
                else:
                    # Handle release logic
                    if active_key == 'A':
                        release_held_key('A')
                        print("Key 'A' released. Executing counter-tap.")
                        hold_key('D')
                        time.sleep(counter_tap_hold_time / 1000.0)  # Counter-tap delay
                        release_held_key('D')
                        active_key = None
                    elif active_key == 'D':
                        release_held_key('D')
                        print("Key 'D' released. Executing counter-tap.")
                        hold_key('A')
                        time.sleep(counter_tap_hold_time / 1000.0)  # Counter-tap delay
                        release_held_key('A')
                        active_key = None

                    # Ensure previous key is re-held if still physically pressed
                    if previous_key == 'A' and a_pressed:
                        hold_key('A')
                        active_key = 'A'
                    elif previous_key == 'D' and d_pressed:
                        hold_key('D')
                        active_key = 'D'

            time.sleep(0.001)  # Short sleep for better responsiveness

        except KeyboardInterrupt:
            print("Exiting...")
            break

if __name__ == "__main__":
    main()
