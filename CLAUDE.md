@AGENTS.md

## Claude Code specifics

- Build artifact is `build/libmimmo.so`; the public entry point is `include/mimmo/api.hpp`.
  Consumers compile with `nvc++ -acc -I<repo>/include` and link `-L<repo>/build -lmimmo`.
- Run the narrowest test that covers a change first:
  `ctest --test-dir build -R <test_name> --output-on-failure`. Run the full suite before
  declaring a change finished.
- When `nvc++`/NVHPC is unavailable, configure with `-DOPENACC=OFF` so the non-OpenACC path
  still builds and tests. A change is not verified until both configurations build.
- Read `CONTRIBUTING.md` before proposing changes to project structure or conventions.
- Detailed rules for dual-memory and OpenACC code live in `.claude/rules/openacc.md` and
  load automatically when you touch matching files.
- Git usage is blocked at the tool level (see `.claude/settings.json`), not just requested
  in AGENTS.md, so it's enforced even if Claude forgets the instruction. The `git clone`
  line in `README.md` is instructions for humans, not something to execute.
- The sandboxed Bash tool is enabled and restricted to the project directory (see
  `.claude/settings.json`). Enforced by the OS, not by instructions, so it holds for
  subagents too.
