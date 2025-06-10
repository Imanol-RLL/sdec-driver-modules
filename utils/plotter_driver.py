import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation

DEVICE_PATH = "/dev/control_gpio"
READ_INTERVAL = 0.1  # seconds

def get_gpio_value():
    """Reads the current signal from the GPIO driver."""
    with open(DEVICE_PATH, 'r') as f:
        line = f.readline().strip()
    return int(line.split(': ')[1])

def set_gpio_pin(pin_choice):
    """Writes the selected signal pin to the driver."""
    with open(DEVICE_PATH, 'w') as f:
        f.write(str(pin_choice))

def prompt_user_choice():
    """Prompts user for GPIO selection."""
    choices = {
        "1": "GPIO_A",
        "2": "GPIO_B"
    }
    print("Choose a GPIO pin to monitor:")
    for k, v in choices.items():
        print(f"{k}: {v}")
    while True:
        choice = input("Enter 1 or 2: ").strip()
        if choice in choices:
            return int(choice)
        print("Invalid selection. Try again.")

def main():
    pin = prompt_user_choice()
    set_gpio_pin(pin)

    times, signals = [], []
    start_time = time.time()

    fig, ax = plt.subplots()
    line, = ax.plot([], [], 'r-', lw=2, label=f"GPIO Signal {pin}")  # Red line
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Signal Level")
    ax.set_title(f"Live GPIO Signal {pin}")
    ax.legend()
    ax.set_ylim(-0.2, 1.2)
    ax.set_xlim(0, 10)

    def update(frame):
        current_time = time.time() - start_time
        value = get_gpio_value()
        times.append(current_time)
        signals.append(value)

        # Keep last 100 data points
        if len(times) > 100:
            times.pop(0)
            signals.pop(0)

        line.set_data(times, signals)
        ax.set_xlim(times[0], times[-1] + 0.1)
        return line,

    ani = animation.FuncAnimation(fig, update, interval=READ_INTERVAL * 1000)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
