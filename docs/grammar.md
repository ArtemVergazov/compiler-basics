$$
\begin{align}
    [\text{Prog}] &\to [\text{Stmt}]^* \\

    [\text{Stmt}] &\to
    \begin{cases}
        \text{exit}\text{(}[\text{Expr}]\text{)}\text{;} \\
        \text{let} \space \text{identifier} \space \text{=} \space [\text{Expr}]\text{;}
    \end{cases} \\

    [\text{Expr}] &\to
    \begin{cases}
        [\text{Term}] \\
        [\text{BinExpr}]
    \end{cases} \\

    [\text{BinExpr}] &\to
    \begin{cases}
        [\text{Expr}] \space \text{*} \space [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] \space \text{/} \space [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] \space \text{+} \space [\text{Expr}] & \text{prec} = 0 \\
        [\text{Expr}] \space \text{-} \space [\text{Expr}] & \text{prec} = 0
    \end{cases} \\

    [\text{Term}] &\to
    \begin{cases}
        \text{intLiteral} \\
        \text{identifier} \\
        \text{(}[\text{Expr}]\text{)}
    \end{cases}
\end{align}
$$
