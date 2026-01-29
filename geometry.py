"""Geometry utilities for beam intersection with a moving ball."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, Optional

import math


@dataclass
class Interval:
    start: float
    end: float

    def intersect(self, other: "Interval") -> Optional["Interval"]:
        start = max(self.start, other.start)
        end = min(self.end, other.end)
        if start <= end:
            return Interval(start, end)
        return None


def _linear_interval(a: float, b: float) -> Optional[Interval]:
    """Solve a * t + b in [0,1] -> interval of t."""
    eps = 1e-12
    if abs(a) < eps:
        if 0.0 <= b <= 1.0:
            return Interval(-math.inf, math.inf)
        return None
    t0 = (0.0 - b) / a
    t1 = (1.0 - b) / a
    return Interval(min(t0, t1), max(t0, t1))


def _quadratic_intervals(a: float, b: float, c: float) -> list[Interval]:
    """Solve a*t^2 + b*t + c <= 0 for intervals."""
    eps = 1e-12
    if abs(a) < eps:
        if abs(b) < eps:
            if c <= 0:
                return [Interval(-math.inf, math.inf)]
            return []
        t = -c / b
        if b > 0:
            return [Interval(-math.inf, t)]
        return [Interval(t, math.inf)]

    disc = b * b - 4 * a * c
    if disc < 0:
        return []
    root = math.sqrt(max(disc, 0.0))
    t1 = (-b - root) / (2 * a)
    t2 = (-b + root) / (2 * a)
    t_low, t_high = sorted((t1, t2))
    if a > 0:
        return [Interval(t_low, t_high)]
    return [Interval(-math.inf, t_low), Interval(t_high, math.inf)]


def _intersect_intervals(intervals: Iterable[Interval], bound: Interval) -> list[Interval]:
    results: list[Interval] = []
    for interval in intervals:
        match = interval.intersect(bound)
        if match:
            results.append(match)
    return results


def _merge_intervals(intervals: list[Interval]) -> list[Interval]:
    if not intervals:
        return []
    intervals = sorted(intervals, key=lambda i: i.start)
    merged = [intervals[0]]
    for interval in intervals[1:]:
        last = merged[-1]
        if interval.start <= last.end:
            last.end = max(last.end, interval.end)
        else:
            merged.append(interval)
    return merged


def moving_circle_segment_interval(
    ax: float,
    ay: float,
    bx: float,
    by: float,
    x0: float,
    y0: float,
    vx: float,
    vy: float,
    radius: float,
) -> Optional[Interval]:
    """Return the first interval when a moving circle intersects a segment.

    The circle center moves linearly: P(t) = (x0, y0) + (vx, vy) * t.
    The beam is a segment A->B. We solve for distance(P(t), segment) <= radius.
    """
    dx = bx - ax
    dy = by - ay
    denom = dx * dx + dy * dy
    if denom == 0:
        return None

    u0 = ((x0 - ax) * dx + (y0 - ay) * dy) / denom
    u1 = (vx * dx + vy * dy) / denom

    u_interval = _linear_interval(u1, u0)

    line_intervals: list[Interval] = []
    if u_interval:
        # Distance to infinite line using projection u(t).
        # P(t) - (A + u(t) d)
        # Expand squared distance into quadratic.
        # Compute coefficients for distance^2 - R^2 <= 0.
        px = x0 - ax
        py = y0 - ay
        # P(t) - u(t) d
        # u(t) = u0 + u1 t
        # vector = (px + vx t) - (u0 + u1 t) * d
        vx_line = vx - u1 * dx
        vy_line = vy - u1 * dy
        px_line = px - u0 * dx
        py_line = py - u0 * dy

        a = vx_line * vx_line + vy_line * vy_line
        b = 2 * (px_line * vx_line + py_line * vy_line)
        c = px_line * px_line + py_line * py_line - radius * radius
        line_intervals = _quadratic_intervals(a, b, c)
        line_intervals = _intersect_intervals(line_intervals, u_interval)

    # Endpoint intervals (distance to A and B).
    endpoints: list[Interval] = []
    for ex, ey in ((ax, ay), (bx, by)):
        px = x0 - ex
        py = y0 - ey
        a = vx * vx + vy * vy
        b = 2 * (px * vx + py * vy)
        c = px * px + py * py - radius * radius
        endpoints.extend(_quadratic_intervals(a, b, c))

    all_intervals = _merge_intervals(line_intervals + endpoints)
    if not all_intervals:
        return None

    # Only consider intervals at t >= 0.
    for interval in all_intervals:
        if interval.end < 0:
            continue
        start = max(interval.start, 0.0)
        return Interval(start, interval.end)
    return None
