//! Python bindings for fast-exif-rs using PyO3
//! 
//! This module provides Python bindings for the fast-exif-rs library,
//! allowing Python users to access the high-performance EXIF reading capabilities.

use pyo3::prelude::*;
use std::collections::HashMap;
use fast_exif_reader::{FastExifReader, FastExifWriter, FastExifCopier, ExifError};

/// Python wrapper for FastExifReader
#[pyclass]
pub struct PyFastExifReader {
    reader: FastExifReader,
}

#[pymethods]
impl PyFastExifReader {
    /// Create a new FastExifReader instance
    #[new]
    pub fn new() -> Self {
        Self {
            reader: FastExifReader::new(),
        }
    }

    /// Read EXIF data from file path
    pub fn read_file(&mut self, file_path: &str) -> PyResult<HashMap<String, String>> {
        self.reader.read_file(file_path)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
    }

    /// Read EXIF data from bytes
    pub fn read_bytes(&mut self, data: &[u8]) -> PyResult<HashMap<String, String>> {
        self.reader.read_bytes(data)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
    }

    /// Read EXIF data from multiple files in parallel
    pub fn read_files_parallel(&mut self, file_paths: Vec<String>) -> PyResult<Vec<HashMap<String, String>>> {
        self.reader.read_files_parallel(file_paths)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
    }
}

impl Default for PyFastExifReader {
    fn default() -> Self {
        Self::new()
    }
}

/// Python wrapper for FastExifWriter
#[pyclass]
pub struct PyFastExifWriter {
    writer: FastExifWriter,
}

#[pymethods]
impl PyFastExifWriter {
    /// Create a new FastExifWriter instance
    #[new]
    pub fn new() -> Self {
        Self {
            writer: FastExifWriter::new(),
        }
    }

    /// Write EXIF metadata to an image file
    pub fn write_exif(
        &self,
        input_path: &str,
        output_path: &str,
        metadata: HashMap<String, String>,
    ) -> PyResult<()> {
        self.writer.write_exif(input_path, output_path, &metadata)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF writing error: {}", e)))
    }

    /// Write EXIF metadata to image bytes
    pub fn write_exif_to_bytes(
        &self,
        input_data: &[u8],
        metadata: HashMap<String, String>,
    ) -> PyResult<Vec<u8>> {
        self.writer.write_exif_to_bytes(input_data, &metadata)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF writing error: {}", e)))
    }

    /// Copy high-priority EXIF fields from source to target image
    pub fn copy_high_priority_exif(
        &self,
        source_path: &str,
        target_path: &str,
        output_path: &str,
    ) -> PyResult<()> {
        self.writer.copy_high_priority_exif(source_path, target_path, output_path)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF copying error: {}", e)))
    }
}

impl Default for PyFastExifWriter {
    fn default() -> Self {
        Self::new()
    }
}

/// Python wrapper for FastExifCopier
#[pyclass]
pub struct PyFastExifCopier {
    copier: FastExifCopier,
}

#[pymethods]
impl PyFastExifCopier {
    /// Create a new FastExifCopier instance
    #[new]
    pub fn new() -> Self {
        Self {
            copier: FastExifCopier::new(),
        }
    }

    /// Copy high-priority EXIF fields from source to target image
    pub fn copy_high_priority_exif(
        &mut self,
        source_path: &str,
        target_path: &str,
        output_path: &str,
    ) -> PyResult<()> {
        self.copier.copy_high_priority_exif(source_path, target_path, output_path)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF copying error: {}", e)))
    }

    /// Copy all EXIF fields from source to target image
    pub fn copy_all_exif(
        &mut self,
        source_path: &str,
        target_path: &str,
        output_path: &str,
    ) -> PyResult<()> {
        self.copier.copy_all_exif(source_path, target_path, output_path)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF copying error: {}", e)))
    }

    /// Copy specific EXIF fields from source to target image
    pub fn copy_specific_exif(
        &mut self,
        source_path: &str,
        target_path: &str,
        output_path: &str,
        field_names: Vec<String>,
    ) -> PyResult<()> {
        let field_names_str: Vec<&str> = field_names.iter().map(|s| s.as_str()).collect();
        self.copier.copy_specific_exif(source_path, target_path, output_path, &field_names_str)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF copying error: {}", e)))
    }

    /// Get available EXIF fields from source image
    pub fn get_available_fields(&mut self, source_path: &str) -> PyResult<Vec<String>> {
        self.copier.get_available_fields(source_path)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
    }

    /// Get high-priority EXIF fields from source image
    pub fn get_high_priority_fields(&mut self, source_path: &str) -> PyResult<HashMap<String, String>> {
        self.copier.get_high_priority_fields(source_path)
            .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
    }
}

impl Default for PyFastExifCopier {
    fn default() -> Self {
        Self::new()
    }
}

/// Standalone function to read EXIF data from a file
#[pyfunction]
pub fn read_exif_file(file_path: &str) -> PyResult<HashMap<String, String>> {
    let mut reader = FastExifReader::new();
    reader.read_file(file_path)
        .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
}

/// Standalone function to read EXIF data from bytes
#[pyfunction]
pub fn read_exif_bytes(data: &[u8]) -> PyResult<HashMap<String, String>> {
    let mut reader = FastExifReader::new();
    reader.read_bytes(data)
        .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
}

/// Standalone function to read EXIF data from multiple files in parallel
#[pyfunction]
pub fn read_exif_files_parallel(file_paths: Vec<String>) -> PyResult<Vec<HashMap<String, String>>> {
    let mut reader = FastExifReader::new();
    reader.read_files_parallel(file_paths)
        .map_err(|e| PyErr::new::<pyo3::exceptions::PyRuntimeError, _>(format!("EXIF reading error: {}", e)))
}

/// Get library version information
#[pyfunction]
pub fn get_version() -> PyResult<String> {
    Ok(env!("CARGO_PKG_VERSION").to_string())
}

/// Get supported file formats
#[pyfunction]
pub fn get_supported_formats() -> PyResult<Vec<String>> {
    Ok(vec![
        "JPEG".to_string(),
        "CR2".to_string(),
        "NEF".to_string(),
        "ARW".to_string(),
        "RAF".to_string(),
        "SRW".to_string(),
        "PEF".to_string(),
        "RW2".to_string(),
        "ORF".to_string(),
        "DNG".to_string(),
        "HEIF".to_string(),
        "HIF".to_string(),
        "MOV".to_string(),
        "MP4".to_string(),
        "3GP".to_string(),
        "AVI".to_string(),
        "WMV".to_string(),
        "WEBM".to_string(),
        "PNG".to_string(),
        "BMP".to_string(),
        "GIF".to_string(),
        "WEBP".to_string(),
        "MKV".to_string(),
    ])
}

/// Python module definition
#[pymodule]
fn fast_exif_rs_py(_py: Python, m: &Bound<PyModule>) -> PyResult<()> {
    // Add classes
    m.add_class::<PyFastExifReader>()?;
    m.add_class::<PyFastExifWriter>()?;
    m.add_class::<PyFastExifCopier>()?;
    
    // Add standalone functions
    m.add_function(wrap_pyfunction!(read_exif_file, m)?)?;
    m.add_function(wrap_pyfunction!(read_exif_bytes, m)?)?;
    m.add_function(wrap_pyfunction!(read_exif_files_parallel, m)?)?;
    m.add_function(wrap_pyfunction!(get_version, m)?)?;
    m.add_function(wrap_pyfunction!(get_supported_formats, m)?)?;
    
    // Add module metadata
    m.add("__version__", env!("CARGO_PKG_VERSION"))?;
    m.add("__author__", "fast-exif-rs contributors")?;
    m.add("__description__", "Python bindings for fast-exif-rs - Fast EXIF reader and writer")?;
    
    Ok(())
}