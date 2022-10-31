// Find the upper, leftmost, and rightmost vertices
		Vec2 high, left, right;
		if (aPos.y >= bPos.y && aPos.y >= cPos.y) {
			high = aPos;
			left = bPos;
			right = cPos;
		}
		else if (bPos.y >= aPos.y && bPos.y >= cPos.y) {
			high = bPos;
			left = aPos;
			right = cPos;
		}
		else {
			high = cPos;
			left = bPos;
			right = aPos;
		}
		if (left.x > right.x) {
			std::swap(left, right);
		}

		// Consider the left and right lines coming down from the upper vertex / \ 
		// m and q are parameters of the equation x = my + q
		float mLeft = (high.x - left.x) / (high.y - left.y);	// TODO What happens if the line is horizontal?
		float qLeft = -left.y * mLeft + left.x;

		float mRight = (high.x - right.x) / (high.y - right.y);
		float qRight = -right.y * mRight + right.x;

		float mBottom = (right.x - left.x) / (right.y - left.y);
		float qBottom = -left.y * mBottom + left.x;

		const float xLeft = mLeft * (high.y - 0.5f) + qLeft;
		const float xRight = mRight * (high.y - 0.5f) + qRight;
		if (xLeft > xRight) {
			std::swap(left, right);
			std::swap(mLeft, mRight);
			std::swap(qLeft, qRight);
		}
        
        // Examine the internal of the triangle line by line
		const float den = (bPos.y - cPos.y) * (aPos.x - cPos.x) + (cPos.x - bPos.x) * (aPos.y - cPos.y);
		bool changedLine = false;
		for (int r = static_cast<int>(high.y + 0.5f); ; --r) {

			// Find leftmost and rightmost pixels in this line
			const float y = r + 0.5f;
			if (y < high.y) {

				if (y < left.y) {
					if (!changedLine) {
						// Change the line / with the line _
						mLeft = mBottom;
						qLeft = qBottom;
						changedLine = true;
						left = right;	// From this point they are treated as one
					}
					else {
						break;
					}
				}
				if (y < right.y) {
					// Change the line \ with the line _
					mRight = mBottom;
					qRight = qBottom;
					changedLine = true;
					right = left;
				}

				const float xLeft = mLeft * y + qLeft;
				const float xRight = mRight * y + qRight;

				// Take all pixels with center between these two extremes
				for (float x = ceilf(xLeft - 0.5f) + 0.5f; x < xRight; ++x) {
					// New fragment
					// Interpolate the values of the vertices
					const float wa = ((bPos.y - cPos.y) * (x - cPos.x) + (cPos.x - bPos.x) * (y - cPos.y)) / den;
					const float wb = ((cPos.y - aPos.y) * (x - cPos.x) + (aPos.x - cPos.x) * (y - cPos.y)) / den;
					const float wc = 1.f - wa - wb;
					const float z = (wa * a.pos.z + wb * b.pos.z + wc * c.pos.z) / 2.f + 0.5f;
					const Vec3 fragPos{ x, y, z };

					const auto fragAttr = tuple_interpolate(a.attr, b.attr, c.attr, wa, wb, wc);

					auto fragment = std::apply([&fragPos](auto&&... attrs) {
						return Fragment(fragPos, attrs...);
						}, fragAttr);

					fragments.push_back(fragment);
				}
			}
		}