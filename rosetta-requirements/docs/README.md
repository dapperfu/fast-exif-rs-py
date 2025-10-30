# Documentation Index

This directory contains generated documentation for both StrictDoc and Doorstop requirements management systems.

## StrictDoc Documentation

Location: `docs/strictdoc_output/`

- **index.html** - Summary documentation of StrictDoc requirements
- **Source documents**: `docs/requirements/RMS.sdoc` and `docs/requirements/DIR.sdoc`

### Status
- Documents parse successfully
- Known limitation: HTML export has traceability graph issue in StrictDoc 0.14.0
- Documents are valid StrictDoc format and can be viewed as source files

## Doorstop Documentation

Location: `docs/doorstop_output/`

- **index.html** - Summary documentation of Doorstop requirements
- **documents/SYS.html** - System Requirements document
- **documents/SRD.html** - Software Requirements document (if generated)

### Requirements Structure
- **SYS**: 5 System Requirements
- **SRD**: 6 Software Requirements (all linked to parent SYS requirements)
- **SDD**: Structure ready for future requirements

### Viewing Documentation
Open `docs/strictdoc_output/index.html` or `docs/doorstop_output/index.html` in a web browser to view the documentation.

## Source Files

- StrictDoc requirements: `docs/requirements/*.sdoc`
- Doorstop requirements: `doorstop_requirements/*/`

