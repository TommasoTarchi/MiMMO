# Contributing to MiMMO

Thank you for your interest in contributing to MiMMO!

## How to Contribute

### Reporting Issues

- Search existing issues before creating a new one
- Include a minimal reproducible example for bugs
- Specify your compiler version and platform

### Pull Requests

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/your-feature-name`
3. **Make your changes** with clear, descriptive commit messages
4. **Add tests** for new functionality
5. **Ensure tests pass**: `ctest --test-dir build --output-on-failure`
6. **Submit a pull request**

### Code Style

- Follow C++17 best practices
- Use meaningful variable and function names
- Add Doxygen comments for public APIs
- Keep headers self-contained with proper includes
- Run `clang-format` before committing (configuration in `.clang-format`)

### Testing

- All new features should include tests
- Run the full test suite before submitting
- Test on both CPU-only and GPU-enabled builds when possible

### Documentation

- Update the README if behavior changes
- Add Doxygen documentation for new public APIs
- Include examples for new features

## Project Structure

```
MiMMO/
├── include/
│   ├── mimmo/          # Public headers
│   └── private/        # Private templated implementations
├── src/                # Implementation files (if any)
├── tests/              # Unit tests
├── examples/           # Usage examples
└── docs/               # Documentation config files
```

## Questions?

Open an issue for discussion before starting significant changes.
