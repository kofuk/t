#!/usr/bin/env python
from fractions import Fraction

def euclidean_distance(p1, p2):
    return (sum((p[0] - p[1]) ** 2 for p in zip(p1, p2))) ** 0.5

if __name__ == '__main__':
    data = [
        (1, 9),
        (1, 8),
        (2, 8),
        (8, 2),
        (9, 2),
        (9, 1)
    ]

    anchor = [
        (1, 8),
        (2, 8)
    ]

    # -1 to iterate until convergence
    max_iter = -1

    while max_iter == -1 or max_iter > 0:
        if max_iter != -1:
            max_iter -= 1

        # Calculate distance from point to anchor
        distance = []
        for a in anchor:
            d = [euclidean_distance(a, d) for d in data]
            distance.append(d)

        # Find nearest anchor for each point
        near = []
        for i in range(len(distance[0])):
            nearest_index = 0
            nearest_distance = 2147483647
            for j in range(len(distance)):
                d = distance[j][i]
                if d < nearest_distance:
                    nearest_index = j
                    nearest_distance = d
            near.append(nearest_index)

        # Update anchor
        new_anchor = []
        for i in range(len(anchor)):
            anc = [0 for i in anchor[0]]
            points = [coord for nearest, coord in zip(near, data) if nearest == i]
            for p in points:
                for j in range(len(p)):
                    anc[j] += p[j]

            for j in range(len(anc)):
                anc[j] = Fraction(anc[j], len(points))

            new_anchor.append(tuple(anc))

        if anchor == new_anchor:
            break

        anchor = new_anchor

        for anc in new_anchor:
            print('  (', end='')
            for p in anc:
                print(p, end=' ')
            print(')', end='')
        print()
