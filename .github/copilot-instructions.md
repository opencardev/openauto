
## Folder Structure

- `/assets`: Contains images, icons, and other media files.
- `/config`: Contains configuration files and settings.
- `/scripts`: Contains shell scripts and automation scripts.
- `/src`: Contains the source code.
- `/docs`: Contains documentation for the project, including specifications and user guides.
- `/tests`: Contains test cases and testing scripts.
- `/build`: Contains build scripts and related files.

## Coding Standards

- Create new files using the provided header template.
- don't put new files in the include/f1x folder, instead create a new folder in appropriate location.
- For all code, follow the principles of clean code: readability, simplicity, and clarity.
- For C/C++ follow the Google C++ Style Guide.
- For Java follow the Google Java Style Guide.
- For Python follow PEP 8 guidelines.
- For PowerShell scripts follow Microsoft PowerShell Style Guide.
- For shell scripts follow Google Shell Style Guide.
- Prefer using Python 3.x for all new scripts.
- Use comments to explain the purpose of complex code sections.
- Use meaningful variable and function names.
- Maintain consistent indentation (4 spaces for Python, 2 spaces for shell scripts).
- Write modular code with functions to enhance readability and reusability.
- Include error handling to manage exceptions gracefully.
- Write unit tests for critical functions to ensure code reliability.
- Use version control (e.g., Git) to track changes and collaborate effectively.
- Document code with docstrings and comments to explain functionality.
- Regularly review and refactor code to improve performance and maintainability.
- Follow best practices for security, such as validating user input and managing sensitive data appropriately.
- Use virtual environments for Python projects to manage dependencies.
- For shell scripts, use `set -e` to exit on errors and `set -u` to treat unset variables as errors.
- Use logging instead of print statements for better tracking and debugging.
- Ensure code is compatible with the target environment (e.g., Raspberry Pi OS version).
- Follow the DRY (Don't Repeat Yourself) principle to avoid code duplication.
- Use consistent naming conventions (e.g., snake_case for variables and functions in Python, kebab-case for shell scripts).
- Regularly update dependencies to their latest stable versions.
- Use linters (e.g., pylint for Python, shellcheck for shell scripts) to enforce coding standards.
- Write clear and concise commit messages that describe the changes made.
- Use branching strategies (e.g., feature branches, pull requests) for collaborative development.
- Conduct code reviews to ensure quality and adherence to standards.
- Maintain a changelog to document significant changes and updates to the project.
- Use existing libraries and frameworks where appropriate to avoid reinventing the wheel.
- Ensure the project is well-documented, including setup instructions, usage guides, and contribution guidelines.
- Use British English for all documentation and comments.
- Ensure all scripts are executable and have the appropriate shebang lines.
- Test scripts on a clean installation of the target OS to ensure compatibility.
- Use environment variables for configuration settings that may change between deployments.
- Avoid hardcoding values; use configuration files or constants instead.
- Ensure scripts are idempotent, meaning they can be run multiple times without causing unintended side effects.
- Use consistent file naming conventions (e.g., lowercase with hyphens for shell scripts,
    underscores for Python files).
- For shell scripts, use double quotes for strings unless single quotes are necessary.
- Avoid using deprecated functions or libraries; always use the latest recommended alternatives.
- Ensure all external dependencies are clearly documented and included in a requirements file (e.g., requirements.txt for Python).
- Use continuous integration (CI) tools to automate testing and deployment processes.
- use loose coupling and high cohesion in code design to enhance maintainability.
- Ensure all code adheres to the principles of clean code, focusing on readability, simplicity, and clarity.
- Regularly back up the codebase to prevent data loss.
- Use consistent formatting for code blocks, including line length limits (e.g., 79 characters for Python).
- Ensure all scripts and code files have appropriate licenses and attributions.
- Use type hints in Python for better code clarity and error checking.
- Avoid using global variables; prefer passing parameters to functions.
- consider performance implications when writing code, especially for resource-constrained environments like Raspberry Pi.
- Consider Queuing long-running tasks to avoid blocking the main thread.
- Use dependency injection where appropriate to enhance testability and flexibility.
- Ensure all user-facing messages are clear, concise, and free of jargon.
- Consider Modern Python features and libraries to improve code efficiency and readability.
- Consider Event-driven programming for handling asynchronous tasks.
- Use design patterns where appropriate to solve common problems in a standardized way.
- Consider using type checking tools (e.g., mypy for Python) to catch type-related errors early.
- Use profiling tools to identify and optimize performance bottlenecks.
- When creating APIs, follow RESTful principles for consistency and scalability.
- Ensure the API follows OpenAPI/Swagger specifications for better documentation and usability.
- Create reusable components and libraries to avoid code duplication.   
- Create unit tests for critical functions to ensure code reliability.
- Use integration tests to verify that different parts of the system work together as expected.
- Use caching strategies to improve performance for frequently accessed data.
- For new files do not use the existing header information, instead use the following template:
[]: # ```plaintext
[]: #  * Project: OpenAuto
[]: #  * This file is part of openauto project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  openauto is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  openauto is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
[]: # ```