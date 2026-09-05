#!/usr/bin/env bash
# Blocks any Bash tool call that invokes `git`, per AGENTS.md: "Do not use git."
# Reads the PreToolUse hook JSON payload from stdin.

CMD=$(cat | jq -r '.tool_input.command // empty')

if echo "$CMD" | grep -Eq '(^|[;&|]\s*)git\b'; then
  echo "Blocked: this project's AGENTS.md says not to use git. Ask the user to run git commands themselves if one is needed." >&2
  exit 2
fi

exit 0
