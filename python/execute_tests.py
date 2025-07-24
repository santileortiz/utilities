from geometry import *
from translation import *

def geometry_tests ():
    test_push('Geometry')

    p1 = Point(0, 0)
    p2 = Point(8, 0)

    p3 = Point(0, 8)
    p4 = Point(8, 8)

    s1 = Segment(p1, p2)
    s2 = Segment(p3, p4)

    s3 = Segment(p1, p3)
    s4 = Segment(p2, p4)

    d1 = Segment(p1, p4)
    d2 = Segment(p2, p3)

    points_collinear_test (p1, p1, p1, True)
    points_collinear_test (p1, p1, Point(5,0), True)
    points_collinear_test (p1, p2, Point(5,0), True)
    points_collinear_test (p1, p2, p3, False)

    test(points_between(p1, p2, Point(4,0)), True)

    segments_intersect_test (s1, s2, False)
    segments_intersect_test (s3, s4, False)

    segments_intersect_test (s1, s3, True)
    segments_intersect_test (s1, d1, True)
    segments_intersect_test (s1, d2, True)
    segments_intersect_test (s1, s4, True)

    segments_intersect_test (s2, s3, True)
    segments_intersect_test (s2, d1, True)
    segments_intersect_test (s2, d2, True)
    segments_intersect_test (s2, s4, True)

    segments_intersect_test (d1, d2, True)

    points_centroid_test ([p1], p1)
    points_centroid_test ([p1, p1, p1, p1], p1)
    points_centroid_test ([p1, p2, p3, p4], Point(4, 4))
    points_centroid_test ([p1, p2], Point(4, 0))
    points_centroid_test ([p1, p3], Point(0, 4))

    points_min_test ([p1, p2, p3, p4], p1)

    point_get_quadrant_test (Point( 1, 0), Point(0, 0), 1)
    point_get_quadrant_test (Point( 1, 1), Point(0, 0), 1)
    point_get_quadrant_test (Point( 0, 1), Point(0, 0), 2)
    point_get_quadrant_test (Point(-1, 1), Point(0, 0), 2)
    point_get_quadrant_test (Point(-1, 0), Point(0, 0), 3)
    point_get_quadrant_test (Point(-1,-1), Point(0, 0), 3)
    point_get_quadrant_test (Point( 0,-1), Point(0, 0), 4)
    point_get_quadrant_test (Point( 1,-1), Point(0, 0), 4)

    o = points_centroid ([p1, p2, p3, p4])
    points_sort_test ([p1, p2, p3, p4], o, [p4, p3, p1, p2])
    points_sort_test ([p1, p2, p3, p4], o, [p2, p1, p3, p4], reverse=True)
    points_sort_test ([p1, Point(8, 4), p2, p4], o, [Point(8, 4), p4, p1, p2])
    points_sort_test ([p1, Point(8, 4), p2, p4], o, [p2, p1, p4, Point(8, 4)], reverse=True)

    test_pop()

def translation_tests():
    test_push('Translation')

    import tempfile
    import os
    from pathlib import Path

    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        (temp_path / "config.en.xml").write_text("English config")
        (temp_path / "config.es.xml").write_text("Spanish config")
        (temp_path / "readme.en.txt").write_text("English readme")
        (temp_path / "readme.es.txt").write_text("Spanish readme")

        get_available_languages_test(temp_dir, {'en', 'es'})

        set_default_language(temp_dir, 'en')

        test((temp_path / "config.xml").read_text(), "English config")
        test((temp_path / "readme.txt").read_text(), "English readme")

        set_default_language(temp_dir, 'es')

        test((temp_path / "config.xml").read_text(), "Spanish config")
        test((temp_path / "readme.txt").read_text(), "Spanish readme")

    test_pop()

if __name__ == "__main__":
    geometry_tests()
    translation_tests()