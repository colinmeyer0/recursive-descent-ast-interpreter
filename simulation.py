"""Simulation engine for beam intervals and Monte Carlo experiments."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import math
import numpy as np

from config import Beam, Shot, SimulationConfig
from geometry import moving_circle_segment_interval
from sensor import BeamInterval, Event, apply_sensor_model, polling_events


@dataclass
class ShotResult:
    shot: Shot
    intervals: list[BeamInterval]
    events: list[Event]
    edge_times: dict[int, tuple[float, float]]


def compute_beam_intervals(shot: Shot, beams: list[Beam], radius: float) -> list[BeamInterval]:
    intervals: list[BeamInterval] = []
    for beam in beams:
        interval = moving_circle_segment_interval(
            beam.ax,
            beam.ay,
            beam.bx,
            beam.by,
            shot.x0,
            shot.y0,
            shot.vx,
            shot.vy,
            radius,
        )
        if interval is None:
            continue
        intervals.append(BeamInterval(beam.beam_id, interval.start, interval.end))
    return intervals


def simulate_shot(
    shot: Shot,
    config: SimulationConfig,
    rng: np.random.Generator,
    poll_rate_hz: float,
) -> ShotResult:
    intervals = compute_beam_intervals(shot, list(config.beams), config.ball_radius)
    events, edge_times = apply_sensor_model(intervals, config.sensor, rng)

    if poll_rate_hz > 0:
        t_end = 0.0
        for times in edge_times.values():
            t_end = max(t_end, times[1])
        t_end += 1.0 / poll_rate_hz
        events = polling_events(config.beams, edge_times, poll_rate_hz, t_end)

    return ShotResult(shot=shot, intervals=intervals, events=events, edge_times=edge_times)


def sample_shot(
    rng: np.random.Generator,
    config: SimulationConfig,
    speed_min: float,
    speed_max: float,
    angle_min: Optional[float],
    angle_max: Optional[float],
    angle_sigma: Optional[float],
) -> Shot:
    speed = rng.uniform(speed_min, speed_max)
    if angle_min is not None and angle_max is not None:
        theta = rng.uniform(angle_min, angle_max)
    elif angle_sigma is not None:
        theta = rng.normal(0.0, angle_sigma)
    else:
        theta = 0.0

    vx = speed * math.cos(theta)
    vy = speed * math.sin(theta)

    half_w = config.goal_width / 2.0
    margin = config.ball_radius * 1.1
    y0 = rng.uniform(-half_w + margin, half_w - margin)

    x0 = min(config.beam_x1, config.beam_x2) - 0.05
    return Shot(x0=x0, y0=y0, vx=vx, vy=vy)


def y_at_beam(shot: Shot, x_beam: float) -> Optional[float]:
    if shot.vx == 0:
        return None
    t = (x_beam - shot.x0) / shot.vx
    return shot.y0 + shot.vy * t
