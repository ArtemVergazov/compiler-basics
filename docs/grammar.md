$$
\begin{align}
    [\text{Prog}] &\to [\text{Stmt}]^* \\

    [\text{Stmt}] &\to
    \begin{cases}
        \text{exit}([\text{Expr}]); \\
        \text{let} \space \text{identifier} \space = \space [\text{Expr}]; \\
        [\text{IfBranch}] \space [\text{ElifBranch}]^* \space [\text{ElseBranch}]^? \\
        [\text{Scope}]
    \end{cases} \\

    [\text{IfBranch}] &\to \text{if} \space ([\text{Expr}]) \space [\text{Scope}] \\

    [\text{ElifBranch}] &\to \text{elif} \space ([\text{Expr}]) \space [\text{Scope}] \\

    [\text{ElseBranch}] &\to \text{else} \space [\text{Scope}] \\

    [\text{Scope}] &\to \text{\{} \space [\text{Stmt}]^* \space \text{\}} \\

    [\text{Expr}] &\to
    \begin{cases}
        [\text{Term}] \\
        [\text{BinExpr}]
    \end{cases} \\

    [\text{BinExpr}] &\to
    \begin{cases}
        [\text{Expr}] \space * \space [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] \space / \space [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] \space + \space [\text{Expr}] & \text{prec} = 0 \\
        [\text{Expr}] \space - \space [\text{Expr}] & \text{prec} = 0
    \end{cases} \\

    [\text{Term}] &\to
    \begin{cases}
        \text{intLiteral} \\
        \text{identifier} \\
        ([\text{Expr}])
    \end{cases}
\end{align}
$$
