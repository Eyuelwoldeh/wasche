# Contributing to Wasche

Thanks for your interest! This is a personal project but I'm happy to accept contributions.

## Getting Started

1. Fork the repo
2. Clone your fork
3. Create a branch for your feature/fix
4. Make your changes
5. Test them
6. Submit a pull request

## What Needs Work?

Check out `docs/DEV_NOTES.md` for my ongoing TODO list. Some areas that need help:

### High Priority
- [ ] Power optimization for battery-powered nodes
- [ ] Better error handling in serial communication
- [ ] Checksum validation implementation
- [ ] Web dashboard/frontend

### Medium Priority
- [ ] Mobile app (React Native maybe?)
- [ ] Push notifications
- [ ] Better calibration tools
- [ ] Usage analytics and graphs

### Low Priority
- [ ] Support for other accelerometers (MPU6050, etc.)
- [ ] Docker containers for easy deployment
- [ ] CI/CD pipeline
- [ ] Better documentation

## Code Style

### C Code (Firmware)
- Keep it readable - embedded code doesn't have to be cryptic
- Comment non-obvious stuff
- Use descriptive variable names
- Follow existing style (spaces, braces, etc.)

### Python Code (Backend)
- Follow PEP 8 (mostly)
- Use type hints where helpful
- Docstrings for functions
- Keep it simple - readability > cleverness

### SQL
- Use lowercase for keywords
- Meaningful table/column names
- Comment complex queries

## Testing

Before submitting:
- Test firmware on actual hardware if possible (or at least compile it)
- Test backend API endpoints with `test_api.py`
- Check that database migrations don't break existing data
- Make sure nothing crashes

## Questions?

Feel free to open an issue or email me: woldeh1@stolaf.edu

I'm usually pretty responsive unless it's finals week lol.

---

## Notes

This is a learning project for me, so the code isn't perfect. If you see something that could be improved, let me know! I'm always trying to get better.

Also, if you're deploying this in a real dorm/apartment building, please:
1. Get permission first
2. Make sure it's secure (the default password is... not great)
3. Consider privacy implications
4. Share what you learned!
