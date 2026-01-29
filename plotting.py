"""Plotting utilities for batch results and single-shot visualization."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import math

import numpy as np


@dataclass
class BatchMetrics:
    angles: np.ndarray
    lateral: np.ndarray
    errors: dict[str, np.ndarray]
    ratios: Optional[np.ndarray]


def plot_batch(metrics: BatchMetrics) -> None:
    import matplotlib.pyplot as plt

    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    ax1, ax2, ax3, ax4 = axes.flatten()

    for name, err in metrics.errors.items():
        ax1.scatter(metrics.angles, err, label=name, alpha=0.6)
    ax1.set_xlabel("theta_true (rad)")
    ax1.set_ylabel("percent error")
    ax1.legend()
    ax1.grid(True)

    for name, err in metrics.errors.items():
        ax2.scatter(metrics.lateral, err, label=name, alpha=0.6)
    ax2.set_xlabel("lateral position (m)")
    ax2.set_ylabel("percent error")
    ax2.legend()
    ax2.grid(True)

    for name, err in metrics.errors.items():
        ax3.hist(err[~np.isnan(err)], bins=40, alpha=0.6, label=name)
    ax3.set_xlabel("percent error")
    ax3.set_ylabel("count")
    ax3.legend()

    if metrics.ratios is not None:
        scatter = ax4.scatter(metrics.angles, metrics.ratios, c=metrics.lateral, cmap="viridis", alpha=0.6)
        ax4.set_xlabel("theta_true (rad)")
        ax4.set_ylabel("ratio r")
        plt.colorbar(scatter, ax=ax4, label="lateral position (m)")
    else:
        ax4.axis("off")

    fig.tight_layout()
    plt.show()


def animate_single_shot(
    positions: np.ndarray,
    times: np.ndarray,
    beam_segments: list[tuple[float, float, float, float]],
    goal_width: float,
    goal_depth: float,
    ball_radius: float,
    true_speed: float,
    est_speeds: dict[str, float],
    edge_times: dict[int, tuple[float, float]],
) -> None:
    import matplotlib.pyplot as plt
    from matplotlib.animation import FuncAnimation

    fig, ax = plt.subplots(figsize=(6, 4))
    half_w = goal_width / 2.0

    ax.set_xlim(-0.1, goal_depth + 0.1)
    ax.set_ylim(-half_w - 0.05, half_w + 0.05)
    ax.set_aspect("equal")

    goal_rect = plt.Rectangle((0, -half_w), goal_depth, goal_width, fill=False)
    ax.add_patch(goal_rect)

    beam_lines = []
    for beam in beam_segments:
        line, = ax.plot([beam[0], beam[2]], [beam[1], beam[3]], color="green", linewidth=2)
        beam_lines.append(line)

    ball = plt.Circle((positions[0, 0], positions[0, 1]), ball_radius, color="blue")
    ax.add_patch(ball)

    text = ax.text(0.02, 0.98, "", transform=ax.transAxes, va="top")

    def update(frame: int):
        x, y = positions[frame]
        ball.center = (x, y)

        t = times[frame]
        for idx, beam in enumerate(beam_segments, start=1):
            fall, rise = edge_times.get(idx, (math.inf, -math.inf))
            broken = fall <= t <= rise
            beam_lines[idx - 1].set_color("red" if broken else "green")

        lines = [f"true v={true_speed:.2f} m/s"]
        for name, speed in est_speeds.items():
            lines.append(f"{name} v={speed:.2f} m/s")
        text.set_text("\n".join(lines))
        return ball, *beam_lines, text

    anim = FuncAnimation(fig, update, frames=len(times), interval=50, blit=False)
    plt.show()
    return anim
