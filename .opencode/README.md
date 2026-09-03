# Pludux OpenCode team

OpenCode discovers the project agents in `.opencode/agents/`. Start OpenCode in
the repository root and select `team-lead` as the primary agent, or address a
specialist directly with `@agent-name`.

## Model routing

| Agent | Model | Purpose |
| --- | --- | --- |
| `team-lead` | `openai/gpt-5.6-terra` | Balanced coordination and routine work |
| `product-owner` | `openai/gpt-5.4-mini` | Low-cost requirements analysis |
| `technical-fellow` | `openai/gpt-5.6-sol` | Selective technical-direction and correctness escalation |
| `cpp-engineer` | `openai/gpt-5.6-terra` | Balanced implementation |
| `qa-engineer` | `openai/gpt-5.4-mini` | Low-cost test analysis and verification |
| `technical-writer` | `openai/gpt-5.4-mini` | Low-cost documentation work |

These IDs assume the OpenAI provider is configured. Run:

```text
opencode models
```

If an ID is unavailable, replace only the `model:` value in the relevant agent
file using a listed `provider/model-id`. Keep the role and permission boundaries
unchanged while comparing models. A sensible substitution policy is:

- Technical Fellow: strongest reasoning/coding model available.
- Team Lead and C++ Engineer: balanced coding model.
- Product Owner, QA Engineer, and Technical Writer: economical mini model.

## Typical usage

```text
@product-owner define acceptance criteria for portfolio-level exposure limits
@technical-fellow review this execution-order design for look-ahead bias
@qa-engineer verify the current diff against the accepted criteria
@technical-writer document the verified behavior
```

For an end-to-end task, remain with `team-lead`; it will use specialists only
when their contribution justifies the extra model call.

## Cost evaluation

For the first 20 representative tasks, record:

- task outcome and whether tests passed;
- agents invoked and repair loops;
- input/output tokens and cost by agent;
- elapsed time;
- defects found after delivery.

Compare this with a single-strong-model baseline. Change routing only when the
same quality bar is met at lower cost or latency.
