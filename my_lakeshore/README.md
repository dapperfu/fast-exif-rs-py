# My Lakeshore Subaru Contact Form Automation

This project contains a Selenium script to automate filling out the contact form on the My Lakeshore Subaru website.

## Features

- Automated form filling with proper error handling
- Support for both Email and Phone contact methods
- Configurable headless mode for background execution
- Comprehensive logging and status messages
- Type hints and documentation following numpy style guidelines

## Requirements

- Python 3.8+
- Chrome browser
- ChromeDriver (automatically managed by webdriver-manager)

## Installation

1. Create and activate a virtual environment:
```bash
make venv
source venv/bin/activate
```

2. Install dependencies:
```bash
make install
```

## Usage

### Basic Usage

Run the script with default sample data:
```bash
make test
```

### Custom Usage

Edit the `main()` function in `contact_form_automation.py` to customize the form data:

```python
sample_data = {
    "first_name": "Your First Name",
    "last_name": "Your Last Name", 
    "contact_method": "Email",  # or "Phone"
    "email": "your.email@example.com",
    "phone": "555-123-4567",
    "comments": "Your message here"
}
```

### Programmatic Usage

```python
from contact_form_automation import MyLakeshoreSubaruContactForm

form_handler = MyLakeshoreSubaruContactForm()
form_handler.run_contact_form_submission(
    first_name="John",
    last_name="Doe",
    contact_method="Email",
    email="john.doe@example.com",
    phone="555-123-4567",
    comments="I'm interested in your Subaru vehicles.",
    headless=False  # Set to True for headless mode
)
```

## Form Fields

The script fills out the following form fields:

- **First Name** (required)
- **Last Name** (required)
- **Contact Method** (required) - "Email" or "Phone"
- **Email** (optional)
- **Phone** (optional)
- **Comments** (optional)

## Error Handling

The script includes comprehensive error handling for:
- Timeout exceptions when waiting for elements
- Missing form elements
- Network connectivity issues
- Browser driver initialization problems

## Headless Mode

To run the script without opening a browser window, set `headless=True` in the script or modify the `run_contact_form_submission()` call.

## Troubleshooting

1. **ChromeDriver issues**: The script uses webdriver-manager to automatically handle ChromeDriver installation
2. **Element not found**: The website structure may have changed - check the form field names
3. **Timeout errors**: Increase the wait time in the WebDriverWait initialization

## License

This script is provided as-is for educational and automation purposes. Please ensure you comply with the website's terms of service when using this automation.
