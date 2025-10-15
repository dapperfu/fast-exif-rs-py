#!/usr/bin/env python3
"""
Selenium script to fill out the My Lakeshore Subaru contact form.

This script automates filling out the contact form on the My Lakeshore Subaru website
and submits it. It includes proper error handling and waits for elements to load.

Author: Claude Sonnet 4 (claude-3-5-sonnet-20241022)
Generated via Cursor IDE (cursor.sh) with AI assistance
"""

import time
from typing import Optional
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import Select
from selenium.common.exceptions import TimeoutException, NoSuchElementException
from selenium.webdriver.chrome.options import Options
from webdriver_manager.chrome import ChromeDriverManager
from webdriver_manager.firefox import GeckoDriverManager
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.firefox.service import Service as FirefoxService
from selenium.webdriver.firefox.options import Options as FirefoxOptions


class MyLakeshoreSubaruContactForm:
    """Class to handle filling out the My Lakeshore Subaru contact form."""
    
    def __init__(self, headless: bool = False) -> None:
        """
        Initialize the contact form handler.
        
        Parameters
        ----------
        headless : bool, optional
            Whether to run the browser in headless mode, by default False
        """
        self.driver: Optional[webdriver.Remote] = None
        self.wait: Optional[WebDriverWait] = None
        self.headless = headless
        
    def setup_driver(self) -> None:
        """Set up the WebDriver with appropriate options, trying Chrome first, then Firefox."""
        try:
            # Try Chrome first
            print("Attempting to set up Chrome WebDriver...")
            chrome_options = Options()
            
            if self.headless:
                chrome_options.add_argument("--headless")
                
            # Additional options for better compatibility
            chrome_options.add_argument("--no-sandbox")
            chrome_options.add_argument("--disable-dev-shm-usage")
            chrome_options.add_argument("--disable-gpu")
            chrome_options.add_argument("--window-size=1920,1080")
            chrome_options.add_argument("--disable-blink-features=AutomationControlled")
            chrome_options.add_experimental_option("excludeSwitches", ["enable-automation"])
            chrome_options.add_experimental_option('useAutomationExtension', False)
            
            # Use webdriver-manager to automatically handle ChromeDriver
            service = Service(ChromeDriverManager().install())
            self.driver = webdriver.Chrome(service=service, options=chrome_options)
            self.driver.execute_script("Object.defineProperty(navigator, 'webdriver', {get: () => undefined})")
            print("✓ Chrome WebDriver initialized successfully")
            
        except Exception as chrome_error:
            print(f"Chrome WebDriver failed: {chrome_error}")
            print("Attempting to set up Firefox WebDriver...")
            
            try:
                # Fallback to Firefox
                firefox_options = FirefoxOptions()
                
                if self.headless:
                    firefox_options.add_argument("--headless")
                    
                firefox_options.add_argument("--width=1920")
                firefox_options.add_argument("--height=1080")
                
                service = FirefoxService(GeckoDriverManager().install())
                self.driver = webdriver.Firefox(service=service, options=firefox_options)
                print("✓ Firefox WebDriver initialized successfully")
                
            except Exception as firefox_error:
                print(f"Firefox WebDriver also failed: {firefox_error}")
                raise RuntimeError("Both Chrome and Firefox WebDrivers failed to initialize")
        
        self.wait = WebDriverWait(self.driver, 15)
        
    def navigate_to_contact_page(self) -> None:
        """Navigate to the My Lakeshore Subaru contact page."""
        if not self.driver:
            raise RuntimeError("Driver not initialized. Call setup_driver() first.")
            
        print("Navigating to My Lakeshore Subaru contact page...")
        self.driver.get("https://www.mylakeshoresubaru.com/contact.htm")
        
        # Wait for the page to load
        self.wait.until(EC.presence_of_element_located((By.TAG_NAME, "body")))
        print("Page loaded successfully")
        
        # Debug: Print page title and URL
        print(f"Page title: {self.driver.title}")
        print(f"Current URL: {self.driver.current_url}")
        
        # Handle cookie banners if present
        self.handle_cookie_banner()
        
    def handle_cookie_banner(self) -> None:
        """Handle cookie consent banners that might block form interaction."""
        try:
            # Look for common cookie banner elements
            cookie_selectors = [
                "button:contains('Allow all cookies')",
                "button:contains('Accept')",
                "button:contains('Accept All')",
                "button:contains('I Accept')",
                "button:contains('Agree')",
                "[id*='cookie'] button",
                "[class*='cookie'] button",
                "[class*='consent'] button"
            ]
            
            for selector in cookie_selectors:
                try:
                    if ":contains" in selector:
                        # Use XPath for text content matching
                        text = selector.split(':contains(')[1].rstrip(')')
                        xpath = f"//button[contains(text(), \"{text}\")]"
                        cookie_button = self.driver.find_element(By.XPATH, xpath)
                    else:
                        cookie_button = self.driver.find_element(By.CSS_SELECTOR, selector)
                    
                    if cookie_button.is_displayed():
                        print(f"Found cookie banner button: {cookie_button.text}")
                        cookie_button.click()
                        print("✓ Cookie banner dismissed")
                        time.sleep(1)  # Wait for banner to disappear
                        break
                except NoSuchElementException:
                    continue
                    
        except Exception as e:
            print(f"Cookie banner handling: {e}")
        
    def fill_contact_form(self, 
                         first_name: str,
                         last_name: str,
                         contact_method: str = "Email",
                         email: Optional[str] = None,
                         phone: Optional[str] = None,
                         comments: Optional[str] = None) -> None:
        """
        Fill out the contact form with the provided information.
        
        Parameters
        ----------
        first_name : str
            First name (required)
        last_name : str
            Last name (required)
        contact_method : str, optional
            Contact method - "Email" or "Phone", by default "Email"
        email : str, optional
            Email address, by default None
        phone : str, optional
            Phone number, by default None
        comments : str, optional
            Comments or message, by default None
        """
        if not self.driver:
            raise RuntimeError("Driver not initialized. Call setup_driver() first.")
            
        print("Filling out contact form...")
        
        
        try:
            # Fill first name
            first_name_field = self.wait.until(
                EC.element_to_be_clickable((By.NAME, "contact.firstName"))
            )
            first_name_field.clear()
            first_name_field.send_keys(first_name)
            print(f"✓ First name filled: {first_name}")
            
            # Fill last name
            last_name_field = self.driver.find_element(By.NAME, "contact.lastName")
            last_name_field.clear()
            last_name_field.send_keys(last_name)
            print(f"✓ Last name filled: {last_name}")
            
            # Fill email if provided
            if email:
                email_field = self.driver.find_element(By.NAME, "contact.email")
                email_field.clear()
                email_field.send_keys(email)
                print(f"✓ Email filled: {email}")
            
            # Fill phone if provided
            if phone:
                phone_field = self.driver.find_element(By.NAME, "contact.phone")
                phone_field.clear()
                phone_field.send_keys(phone)
                print(f"✓ Phone filled: {phone}")
            
            # Fill comments if provided
            if comments:
                comments_field = self.driver.find_element(By.NAME, "comments")
                comments_field.clear()
                comments_field.send_keys(comments)
                print(f"✓ Comments filled: {comments}")
                
        except TimeoutException:
            print("❌ Timeout waiting for form elements to load")
            raise
        except NoSuchElementException as e:
            print(f"❌ Form element not found: {e}")
            raise
        except Exception as e:
            print(f"❌ Error filling form: {e}")
            raise
            
    def submit_form(self) -> None:
        """Submit the contact form."""
        if not self.driver:
            raise RuntimeError("Driver not initialized. Call setup_driver() first.")
            
        try:
            # Find and click the submit button - try multiple selectors
            submit_button = None
            
            # Try different selectors for the submit button
            selectors = [
                "input[type='submit']",
                "button[type='submit']", 
                "button:contains('Send Message')",
                "input[value*='Send']",
                "button:contains('Submit')",
                "input[value*='Submit']"
            ]
            
            for selector in selectors:
                try:
                    if ":contains" in selector:
                        # Use XPath for text content matching
                        xpath = f"//button[contains(text(), '{selector.split(':contains(')[1].rstrip(')')}')]"
                        submit_button = self.driver.find_element(By.XPATH, xpath)
                    else:
                        submit_button = self.driver.find_element(By.CSS_SELECTOR, selector)
                    break
                except NoSuchElementException:
                    continue
            
            if not submit_button:
                # Fallback: look for any button with "send" or "submit" in text
                buttons = self.driver.find_elements(By.TAG_NAME, "button")
                for btn in buttons:
                    if btn.text and ("send" in btn.text.lower() or "submit" in btn.text.lower()):
                        submit_button = btn
                        break
            
            if not submit_button:
                raise NoSuchElementException("Could not find submit button")
            
            print("Submitting form...")
            
            # Scroll to the button to ensure it's visible
            self.driver.execute_script("arguments[0].scrollIntoView(true);", submit_button)
            time.sleep(1)
            
            # Try regular click first
            try:
                submit_button.click()
                print("✓ Form submitted successfully")
            except Exception as click_error:
                print(f"Regular click failed: {click_error}")
                print("Trying JavaScript click...")
                
                # Use JavaScript click as fallback
                self.driver.execute_script("arguments[0].click();", submit_button)
                print("✓ Form submitted successfully (via JavaScript)")
            
            # Wait a moment to see if there's a confirmation message
            time.sleep(3)
            
        except TimeoutException:
            print("❌ Timeout waiting for submit button")
            raise
        except Exception as e:
            print(f"❌ Error submitting form: {e}")
            raise
            
    def close_driver(self) -> None:
        """Close the WebDriver."""
        if self.driver:
            self.driver.quit()
            print("✓ Browser closed")
            
    def run_contact_form_submission(self,
                                  first_name: str,
                                  last_name: str,
                                  contact_method: str = "Email",
                                  email: Optional[str] = None,
                                  phone: Optional[str] = None,
                                  comments: Optional[str] = None,
                                  headless: bool = False) -> None:
        """
        Complete workflow to fill and submit the contact form.
        
        Parameters
        ----------
        first_name : str
            First name (required)
        last_name : str
            Last name (required)
        contact_method : str, optional
            Contact method - "Email" or "Phone", by default "Email"
        email : str, optional
            Email address, by default None
        phone : str, optional
            Phone number, by default None
        comments : str, optional
            Comments or message, by default None
        headless : bool, optional
            Whether to run in headless mode, by default False
        """
        try:
            self.headless = headless
            self.setup_driver()
            self.navigate_to_contact_page()
            self.fill_contact_form(first_name, last_name, contact_method, email, phone, comments)
            self.submit_form()
            
        except Exception as e:
            print(f"❌ Error in contact form submission: {e}")
            raise
        finally:
            self.close_driver()


def main() -> None:
    """Main function to demonstrate the contact form submission."""
    # Example usage
    form_handler = MyLakeshoreSubaruContactForm()
    
    # Sample data - replace with actual data
    sample_data = {
        "first_name": "John",
        "last_name": "Doe",
        "contact_method": "Email",
        "email": "john.doe@example.com",
        "phone": "555-123-4567",
        "comments": "I'm interested in learning more about your Subaru vehicles. Please contact me with more information."
    }
    
    try:
        form_handler.run_contact_form_submission(
            first_name=sample_data["first_name"],
            last_name=sample_data["last_name"],
            contact_method=sample_data["contact_method"],
            email=sample_data["email"],
            phone=sample_data["phone"],
            comments=sample_data["comments"],
            headless=False  # Set to True to run without opening browser window
        )
        print("✅ Contact form submission completed successfully!")
        
    except Exception as e:
        print(f"❌ Contact form submission failed: {e}")


if __name__ == "__main__":
    main()
