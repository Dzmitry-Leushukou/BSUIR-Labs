import numpy as np
import pandas as pd
import plotly.graph_objects as go
import streamlit as st
from scipy.integrate import dblquad
from scipy.stats import chi2

st.set_page_config(page_title="Lab 3", layout="wide")


def independence_test(x, y, bins):
    effective_bins = min(bins, max(2, int(np.sqrt(len(x) / 5))))
    x_edges = np.unique(np.quantile(x, np.linspace(0, 1, effective_bins + 1)))
    y_edges = np.unique(np.quantile(y, np.linspace(0, 1, effective_bins + 1)))
    if len(x_edges) < 3 or len(y_edges) < 3:
        table, x_edges, y_edges = np.histogram2d(x, y, bins=bins)
    else:
        table, x_edges, y_edges = np.histogram2d(x, y, bins=(x_edges, y_edges))
    row_totals = table.sum(axis=1, keepdims=True)
    col_totals = table.sum(axis=0, keepdims=True)
    expected = row_totals @ col_totals / table.sum()
    valid = expected > 0
    statistic = float(np.sum((table[valid] - expected[valid]) ** 2 / expected[valid]))
    active_rows = int(np.count_nonzero(row_totals))
    active_cols = int(np.count_nonzero(col_totals))
    degrees = max(1, (active_rows - 1) * (active_cols - 1))
    p_value = float(chi2.sf(statistic, degrees))
    return table, expected, x_edges, y_edges, statistic, degrees, p_value


def continuous_sample(size, seed):
    generator = np.random.default_rng(seed)
    uniform_radius = np.maximum(generator.random(size), np.finfo(float).tiny)
    uniform_angle = generator.random(size)
    radius = np.sqrt(-2 * np.log(uniform_radius)) / np.sqrt(2)
    angle = 2 * np.pi * uniform_angle
    return radius * np.cos(angle), radius * np.sin(angle)


@st.cache_data
def continuous_theory():
    joint_density = lambda x, y: np.exp(-(x * x + y * y)) / np.pi

    def moment(power_x, power_y):
        return dblquad(
            lambda y, x: x**power_x * y**power_y * joint_density(x, y),
            -np.inf,
            np.inf,
            lambda _: -np.inf,
            lambda _: np.inf,
            epsabs=1e-10,
            epsrel=1e-10,
        )[0]

    mean_x = moment(1, 0)
    mean_y = moment(0, 1)
    second_moment_x = moment(2, 0)
    second_moment_y = moment(0, 2)
    mixed_moment = moment(1, 1)
    variance_x = second_moment_x - mean_x**2
    variance_y = second_moment_y - mean_y**2
    covariance = mixed_moment - mean_x * mean_y
    correlation = covariance / np.sqrt(variance_x * variance_y)
    return {
        "M(X)": float(np.round(mean_x, 10)),
        "M(Y)": float(np.round(mean_y, 10)),
        "D(X)": float(np.round(variance_x, 10)),
        "D(Y)": float(np.round(variance_y, 10)),
        "Cov(X,Y)": float(np.round(covariance, 10)),
        "ρ(X,Y)": float(np.round(correlation, 10)),
    }


def bootstrap_intervals(x, y, level, seed):
    generator = np.random.default_rng(seed)
    sample_size = min(len(x), 2000)
    replicate_count = 300
    estimates = np.empty((replicate_count, 6))
    for index in range(replicate_count):
        selected = generator.integers(0, len(x), sample_size)
        bx = x[selected]
        by = y[selected]
        estimates[index] = (
            np.mean(bx),
            np.mean(by),
            np.var(bx, ddof=1),
            np.var(by, ddof=1),
            np.cov(bx, by, ddof=1)[0, 1],
            np.corrcoef(bx, by)[0, 1],
        )
    names = ["M(X)", "M(Y)", "D(X)", "D(Y)", "Cov(X,Y)", "ρ(X,Y)"]
    center = np.array([np.mean(x), np.mean(y), np.var(x, ddof=1), np.var(y, ddof=1), np.cov(x, y, ddof=1)[0, 1], np.corrcoef(x, y)[0, 1]])
    scale = np.sqrt(sample_size / len(x))
    tail = (1 - level / 100) / 2
    intervals = np.quantile((estimates - center) * scale, [tail, 1 - tail], axis=0).T + center[:, None]
    return {name: tuple(intervals[index]) for index, name in enumerate(names)}


def discrete_sample(size, seed, probabilities):
    generator = np.random.default_rng(seed)
    rows, cols = probabilities.shape
    flat = generator.choice(rows * cols, size=size, p=probabilities.ravel())
    return flat // cols, flat % cols


def continuous_analysis(x, y, level, bins):
    table, expected, x_edges, y_edges, statistic, degrees, p_value = independence_test(x, y, bins)
    values = {
        "M(X)": float(np.mean(x)),
        "M(Y)": float(np.mean(y)),
        "D(X)": float(np.var(x, ddof=1)),
        "D(Y)": float(np.var(y, ddof=1)),
        "Cov(X,Y)": float(np.cov(x, y, ddof=1)[0, 1]),
        "ρ(X,Y)": float(np.corrcoef(x, y)[0, 1]),
    }
    intervals = bootstrap_intervals(x, y, level, seed=913)
    return values, intervals, (table, expected, x_edges, y_edges, statistic, degrees, p_value)


def discrete_theory(probabilities):
    rows, cols = probabilities.shape
    x = np.arange(rows)
    y = np.arange(cols)
    px = probabilities.sum(axis=1)
    py = probabilities.sum(axis=0)
    ex = float(np.sum(x * px))
    ey = float(np.sum(y * py))
    ex2 = float(np.sum(x * x * px))
    ey2 = float(np.sum(y * y * py))
    exy = float(np.sum((x[:, None] * y[None, :]) * probabilities))
    dx = ex2 - ex * ex
    dy = ey2 - ey * ey
    cov = exy - ex * ey
    return {"M(X)": ex, "M(Y)": ey, "D(X)": dx, "D(Y)": dy, "Cov(X,Y)": cov, "ρ(X,Y)": cov / np.sqrt(dx * dy)}


def discrete_analysis(x, y, probabilities, level, bins):
    values = {
        "M(X)": float(np.mean(x)),
        "M(Y)": float(np.mean(y)),
        "D(X)": float(np.var(x, ddof=1)),
        "D(Y)": float(np.var(y, ddof=1)),
        "Cov(X,Y)": float(np.cov(x, y, ddof=1)[0, 1]),
        "ρ(X,Y)": float(np.corrcoef(x, y)[0, 1]),
    }
    intervals = bootstrap_intervals(x, y, level, seed=1871)
    table = np.zeros_like(probabilities, dtype=float)
    for a, b in zip(x, y):
        table[a, b] += 1
    row_totals = table.sum(axis=1, keepdims=True)
    col_totals = table.sum(axis=0, keepdims=True)
    expected = row_totals @ col_totals / len(x)
    valid = expected > 0
    statistic = float(np.sum((table[valid] - expected[valid]) ** 2 / expected[valid]))
    active_rows = int(np.count_nonzero(row_totals))
    active_cols = int(np.count_nonzero(col_totals))
    degrees = max(1, (active_rows - 1) * (active_cols - 1))
    p_value = float(chi2.sf(statistic, degrees))
    return values, intervals, (table, expected, statistic, degrees, p_value)


def format_p_value(value):
    if value == 0.0:
        return "<1e-300"
    if value < 1e-5:
        return f"{value:.3e}"
    return f"{value:.5f}"


def statistic_table(values, theory, intervals):
    rows = []
    for name in values:
        interval = intervals.get(name, (np.nan, np.nan))
        theoretical = theory.get(name, np.nan)
        tested = not np.isnan(interval[0]) and not np.isnan(theoretical)
        decision = "не отвергается" if tested and interval[0] <= theoretical <= interval[1] else "отвергается" if tested else "н/д"
        rows.append({"Показатель": name, "Теория": theoretical, "Оценка": values[name], "Доверительный интервал": f"[{interval[0]:.5f}; {interval[1]:.5f}]" if not np.isnan(interval[0]) else "-", "H₀: теория верна": decision})
    return pd.DataFrame(rows)


def continuous_plots(x, y, bins):
    grid = np.linspace(-3.2, 3.2, 80)
    xx, yy = np.meshgrid(grid, grid)
    density = np.exp(-(xx * xx + yy * yy)) / np.pi
    fig_components = go.Figure()
    component_edges = np.linspace(min(x.min(), y.min()), max(x.max(), y.max()), bins + 1)
    component_density = np.exp(-grid * grid) / np.sqrt(np.pi)
    fig_components.add_trace(go.Histogram(x=x, histnorm="probability density", xbins={"start": component_edges[0], "end": component_edges[-1], "size": component_edges[1] - component_edges[0]}, name="Выборка X", opacity=0.58, marker_color="#6f8f8b"))
    fig_components.add_trace(go.Histogram(x=y, histnorm="probability density", xbins={"start": component_edges[0], "end": component_edges[-1], "size": component_edges[1] - component_edges[0]}, name="Выборка Y", opacity=0.52, marker_color="#bd8566"))
    fig_components.add_trace(go.Scatter(x=grid, y=component_density, mode="lines", name="Теоретическая плотность", line={"color": "#344f4d", "width": 2.5}))
    fig_components.update_layout(
        template="plotly_white",
        barmode="overlay",
        xaxis_title="Значение компоненты",
        yaxis_title="Плотность вероятности",
        margin={"l": 55, "r": 20, "t": 35, "b": 125},
        legend={"orientation": "h", "x": 0.5, "xanchor": "center", "y": -0.3, "yanchor": "top"},
    )
    hist, xe, ye = np.histogram2d(x, y, bins=bins, density=True)
    centers_x = (xe[:-1] + xe[1:]) / 2
    centers_y = (ye[:-1] + ye[1:]) / 2
    fig_joint = go.Figure(data=[
        go.Surface(x=grid, y=grid, z=density, colorscale=[[0, "#547a76"], [1, "#547a76"]], opacity=0.68, name="Теоретическая плотность", showscale=False, showlegend=True),
        go.Surface(x=centers_x, y=centers_y, z=hist.T, colorscale=[[0, "#b87453"], [1, "#b87453"]], opacity=0.68, name="Выборочная плотность", showscale=False, showlegend=True),
    ])
    fig_joint.update_layout(
        title={"text": "Совместная плотность X и Y", "x": 0.02, "xanchor": "left"},
        template="plotly_white",
        showlegend=True,
        legend={"orientation": "h", "x": 0.5, "xanchor": "center", "y": -0.12, "yanchor": "top"},
        scene={
            "xaxis": {"title": {"text": "X"}, "showticklabels": True},
            "yaxis": {"title": {"text": "Y"}, "showticklabels": True},
            "zaxis": {"title": {"text": "Плотность f(x, y)"}, "showticklabels": True},
            "aspectmode": "cube",
        },
        margin={"l": 0, "r": 0, "t": 60, "b": 90},
    )
    return fig_components, fig_joint


def discrete_plots(x, y, probabilities):
    rows, cols = probabilities.shape
    px = probabilities.sum(axis=1)
    py = probabilities.sum(axis=0)
    fig_x = go.Figure([go.Bar(x=np.arange(rows), y=np.bincount(x, minlength=rows) / len(x), name="По выборке", marker_color="#809894"), go.Scatter(x=np.arange(rows), y=px, mode="lines+markers", name="Теория", line={"color": "#a56649", "width": 2})])
    fig_x.update_layout(
        title={"text": "Маргинальное распределение X", "x": 0.02, "xanchor": "left"},
        template="plotly_white",
        xaxis_title="Значение X",
        yaxis_title="Вероятность",
        margin={"l": 55, "r": 20, "t": 60, "b": 90},
        legend={"orientation": "h", "x": 0.5, "xanchor": "center", "y": -0.24, "yanchor": "top"},
    )
    fig_y = go.Figure([go.Bar(x=np.arange(cols), y=np.bincount(y, minlength=cols) / len(y), name="По выборке", marker_color="#809894"), go.Scatter(x=np.arange(cols), y=py, mode="lines+markers", name="Теория", line={"color": "#a56649", "width": 2})])
    fig_y.update_layout(
        title={"text": "Маргинальное распределение Y", "x": 0.02, "xanchor": "left"},
        template="plotly_white",
        xaxis_title="Значение Y",
        yaxis_title="Вероятность",
        margin={"l": 55, "r": 20, "t": 60, "b": 90},
        legend={"orientation": "h", "x": 0.5, "xanchor": "center", "y": -0.24, "yanchor": "top"},
    )
    fig_3d = go.Figure(data=[go.Surface(x=np.arange(cols), y=np.arange(rows), z=probabilities, colorscale=[[0, "#dce6e4"], [1, "#547a76"]], showscale=False)])
    fig_3d.update_layout(
        title={"text": "Теоретическое совместное распределение", "x": 0.02, "xanchor": "left"},
        template="plotly_white",
        scene={
            "xaxis": {"title": {"text": "Значение Y"}, "tickmode": "array", "tickvals": np.arange(cols)},
            "yaxis": {"title": {"text": "Значение X"}, "tickmode": "array", "tickvals": np.arange(rows)},
            "zaxis": {"title": {"text": "Вероятность"}},
            "aspectmode": "cube",
        },
    )
    fig_3d.update_layout(height=520, margin={"l": 20, "r": 20, "t": 70, "b": 20})
    observed = np.zeros_like(probabilities, dtype=float)
    for a, b in zip(x, y):
        observed[a, b] += 1 / len(x)
    fig_hist3d = go.Figure(data=[go.Surface(x=np.arange(cols), y=np.arange(rows), z=observed, colorscale=[[0, "#efe2d9"], [1, "#b87453"]], showscale=False)])
    fig_hist3d.update_layout(
        title={"text": "Выборочная частотная поверхность", "x": 0.02, "xanchor": "left"},
        template="plotly_white",
        scene={
            "xaxis": {"title": {"text": "Значение Y"}, "tickmode": "array", "tickvals": np.arange(cols)},
            "yaxis": {"title": {"text": "Значение X"}, "tickmode": "array", "tickvals": np.arange(rows)},
            "zaxis": {"title": {"text": "Относительная частота"}},
            "aspectmode": "cube",
        },
    )
    fig_hist3d.update_layout(height=520, margin={"l": 20, "r": 20, "t": 70, "b": 20})
    return fig_x, fig_y, fig_3d, fig_hist3d


with st.sidebar:
    st.header("Параметры")
    sample_size = st.number_input("Размер выборки", min_value=100, max_value=500000, value=10000, step=100)
    seed = st.number_input("Начальное значение генератора", min_value=0, max_value=2**31 - 1, value=17, step=1)
    bins = st.slider("Число интервалов", 5, 40, 15)
    level = st.slider("Доверительная вероятность, %", 1, 99, 95)

continuous_tab, discrete_tab = st.tabs(["Непрерывная НСВ, вариант 8", "Дискретная ДСВ"])

with continuous_tab:
    st.subheader("Непрерывное распределение · вариант 8")
    st.latex(r"f_{X,Y}(x,y)=\frac{1}{\pi}e^{-x^2-y^2},\qquad x,y\in\mathbb{R}")
    x, y = continuous_sample(int(sample_size), int(seed))
    values, intervals, test = continuous_analysis(x, y, level, bins)
    theory = continuous_theory()
    st.dataframe(statistic_table(values, theory, intervals), width="stretch", hide_index=True)
    table, expected, x_edges, y_edges, statistic, degrees, p_value = test
    c1, c2 = st.columns(2)
    c1.metric(f"χ² независимости (df={degrees})", f"{statistic:.4f}")
    c2.metric("p-value", format_p_value(p_value))
    alpha = 1 - level / 100
    if p_value >= alpha:
        st.success(f"При α={alpha:.2f} нет оснований отвергнуть гипотезу независимости.")
    else:
        st.warning(f"При α={alpha:.2f} гипотеза независимости отвергается.")
    st.latex(r"f_{X|Y}(x|y)=\frac{f_{X,Y}(x,y)}{f_Y(y)}=\frac{1}{\sqrt{\pi}}e^{-x^2},\qquad f_{Y|X}(y|x)=\frac{f_{X,Y}(x,y)}{f_X(x)}=\frac{1}{\sqrt{\pi}}e^{-y^2}")
    st.caption("H₀: X и Y независимы. Теоретические значения характеристик проверяются по доверительным интервалам.")
    fig_components, fig_joint = continuous_plots(x, y, bins)
    st.subheader("Распределение компонент X и Y")
    st.plotly_chart(fig_components, use_container_width=True)
    st.plotly_chart(fig_joint, use_container_width=True)
    st.subheader("Таблица частот для проверки независимости")
    st.dataframe(pd.DataFrame(table.astype(int)), width="stretch")

with discrete_tab:
    st.subheader("Дискретное распределение")
    st.latex(r"p_{ij}=P(X=i,Y=j),\qquad \sum_i\sum_j p_{ij}=1")
    default_matrix = np.array([[0.05, 0.10, 0.05], [0.10, 0.20, 0.10], [0.05, 0.15, 0.20]])
    matrix_text = st.text_area("Матрица вероятностей pᵢⱼ (строки — X, столбцы — Y)", value="0.05 0.10 0.05\n0.10 0.20 0.10\n0.05 0.15 0.20", height=100, help="Введите числа через пробел. Сумма матрицы должна быть равна 1.")
    try:
        matrix = np.array([[float(value) for value in row.split()] for row in matrix_text.strip().splitlines()], dtype=float)
        valid_matrix = matrix.ndim == 2 and min(matrix.shape) >= 2 and np.all(np.isfinite(matrix)) and np.all(matrix >= 0) and np.isclose(matrix.sum(), 1.0) and np.all(matrix.sum(axis=0) > 0) and np.all(matrix.sum(axis=1) > 0)
    except ValueError:
        matrix = default_matrix
        valid_matrix = False
    if not valid_matrix:
        st.error("Нужна прямоугольная матрица не меньше 2 × 2: вероятности неотрицательны, сумма равна 1, каждая строка и каждый столбец имеют положительную сумму.")
    else:
        dx, dy = discrete_sample(int(sample_size), int(seed), matrix)
        dvalues, dintervals, dtest = discrete_analysis(dx, dy, matrix, level, bins)
        dtheory = discrete_theory(matrix)
        st.dataframe(statistic_table(dvalues, dtheory, dintervals), width="stretch", hide_index=True)
        dtable, dexpected, dstatistic, ddegrees, dp_value = dtest
        c1, c2 = st.columns(2)
        c1.metric(f"χ² независимости (df={ddegrees})", f"{dstatistic:.4f}")
        c2.metric("p-value", format_p_value(dp_value))
        alpha = 1 - level / 100
        if dp_value >= alpha:
            st.success(f"При α={alpha:.2f} нет оснований отвергнуть гипотезу независимости.")
        else:
            st.warning(f"При α={alpha:.2f} гипотеза независимости отвергается.")
        st.caption("H₀: X и Y независимы. Теоретические значения характеристик проверяются по доверительным интервалам.")
        st.latex(r"P(Y=j\mid X=i)=\frac{p_{ij}}{p_{i\cdot}},\quad p_{i\cdot}=\sum_j p_{ij},\qquad P(X=i\mid Y=j)=\frac{p_{ij}}{p_{\cdot j}},\quad p_{\cdot j}=\sum_i p_{ij}")
        st.write("Условные вероятности Y при каждом значении X:")
        conditional_y_given_x = pd.DataFrame(
            matrix / matrix.sum(axis=1, keepdims=True),
            index=[f"X={value}" for value in range(matrix.shape[0])],
            columns=[f"Y={value}" for value in range(matrix.shape[1])],
        )
        st.dataframe(conditional_y_given_x, width="stretch")
        st.write("Условные вероятности X при каждом значении Y:")
        conditional_x_given_y = pd.DataFrame(
            (matrix / matrix.sum(axis=0, keepdims=True)).T,
            index=[f"Y={value}" for value in range(matrix.shape[1])],
            columns=[f"X={value}" for value in range(matrix.shape[0])],
        )
        st.dataframe(conditional_x_given_y, width="stretch")
        fig_x, fig_y, fig_3d, fig_hist3d = discrete_plots(dx, dy, matrix)
        left, right = st.columns(2)
        left.plotly_chart(fig_x, use_container_width=True)
        right.plotly_chart(fig_y, use_container_width=True)
        st.plotly_chart(fig_3d, use_container_width=True)
        st.plotly_chart(fig_hist3d, use_container_width=True)
        st.subheader("Выборочная таблица частот")
        st.dataframe(pd.DataFrame(dtable.astype(int)), width="stretch")
