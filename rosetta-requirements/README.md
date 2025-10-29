# Requirements Tracking Tools Evaluation

This repository provides an evaluation environment for open source requirements tracking tools. Two tools are installed for hands-on evaluation (Doorstop and StrictDoc), while several others are documented for reference.

## Installed Tools

### 1. StrictDoc
**Repository:** https://github.com/strictdoc-project/strictdoc  
**Documentation:** https://strictdoc.readthedocs.io/

Modern documentation and requirements management tool using a custom .sdoc format based on a strict grammar. StrictDoc provides:
- Web-based UI for viewing and editing requirements
- Traceability between requirements at different levels
- Export to multiple formats (HTML, PDF, ReqIF, Excel)
- Built-in support for linking requirements to source code
- Version control friendly text-based format
- Support for requirements coverage and impact analysis

**Key Strengths:**
- Modern, clean web interface
- Strong traceability features
- Active development and maintenance
- Good documentation
- Integration with CI/CD pipelines

### 2. Doorstop
**Repository:** https://github.com/doorstop-dev/doorstop  
**Documentation:** https://doorstop.readthedocs.io/

Requirements management using Markdown and YAML files with built-in traceability and validation.

**Key Features:**
- Text-based format using Markdown for requirement text and YAML for metadata
- Hierarchical document structure
- Link validation between requirements
- Command-line interface
- Export to HTML, Markdown, and other formats
- Git-friendly storage format

**Key Strengths:**
- Simple, lightweight approach
- Easy to learn and use
- Works well with version control
- Extensible through Python API
- Good for smaller to medium projects

## Other Open Source Requirements Tools (Documented)

### 3. Sphinx-Needs
**Repository:** https://github.com/useblocks/sphinx-needs  
**Documentation:** https://sphinx-needs.readthedocs.io/

Extension for Sphinx documentation framework that adds requirements management capabilities.

**Key Features:**
- Integrates requirements into Sphinx documentation
- Supports multiple requirement types (needs, specs, tests, etc.)
- Filtering and searching capabilities
- Traceability matrices
- PlantUML integration for diagrams

**Best For:** Projects already using Sphinx for documentation

### 4. RMTOO (Requirements Management Tool)
**Repository:** https://github.com/florath/rmtoo  
**Documentation:** http://rmtoo.florath.net/

Python-based requirements management with multiple output formats.

**Key Features:**
- Text-based input format
- Dependency management between requirements
- Output to LaTeX, HTML, XML
- Integration with version control
- Constraint checking and validation

**Best For:** Document-centric requirements management with academic or formal documentation needs

### 5. Capella
**Repository:** https://github.com/eclipse/capella  
**Website:** https://www.eclipse.org/capella/

Eclipse-based Model-Based Systems Engineering (MBSE) tool implementing the Arcadia method.

**Key Features:**
- Complete MBSE environment
- Multiple architecture viewpoints
- Requirements import and traceability
- Model-based approach
- SysML-like modeling

**Best For:** Large-scale systems engineering projects requiring full MBSE capabilities

### 6. Papyrus
**Repository:** https://www.eclipse.org/papyrus/  
**Website:** https://www.eclipse.org/papyrus/

Eclipse-based graphical modeling tool with SysML and UML support.

**Key Features:**
- Full SysML and UML support
- Requirements diagram support
- Integration with Eclipse ecosystem
- Customizable modeling environment
- ReqIF import/export

**Best For:** Projects using SysML or requiring tight integration with Eclipse tools

### 7. ReqIF Studio
**Website:** https://www.formalmind.com/en/products/reqif-studio  
**Note:** Community edition available

Tool for working with ReqIF (Requirements Interchange Format) files.

**Key Features:**
- Native ReqIF editing
- Format conversion capabilities
- Requirements import/export
- Validation and checking

**Best For:** Organizations needing ReqIF compatibility for tool interoperability

## Quick Start

### Installation

```bash
# Create virtual environment and install tools
make install
```

### Using StrictDoc

```bash
# Activate virtual environment
source venv/bin/activate

# Create a new StrictDoc project
strictdoc init my-project

# Generate HTML documentation
cd my-project
strictdoc export .

# Start web server for browsing
strictdoc server .
```

### Using Doorstop

```bash
# Activate virtual environment
source venv/bin/activate

# Create a new document
doorstop create REQ ./requirements

# Add a requirement
doorstop add REQ

# Publish to HTML
doorstop publish all ./output
```

## Comparison Summary

| Tool | Format | Complexity | Best Use Case |
|------|--------|------------|---------------|
| StrictDoc | .sdoc | Medium | Modern projects needing web UI and strong traceability |
| Doorstop | Markdown/YAML | Low | Lightweight requirements with Git integration |
| Sphinx-Needs | reStructuredText | Medium | Projects already using Sphinx |
| RMTOO | Custom text | Medium | Academic/formal documentation |
| Capella | Model-based | High | Large systems engineering projects |
| Papyrus | SysML/UML | High | Model-based development with SysML |
| ReqIF Studio | ReqIF XML | Medium | Tool interoperability via ReqIF |

## Environment Details

- Python virtual environment: `venv/`
- Dependencies managed via: `requirements.txt`
- Build automation: `Makefile`

## Makefile Targets

- `make install` - Create venv and install all dependencies
- `make clean` - Remove virtual environment and generated files
- `make venv` - Create Python virtual environment only

## License

This evaluation repository is for research and evaluation purposes. Each tool has its own license - please refer to their respective repositories for licensing information.

