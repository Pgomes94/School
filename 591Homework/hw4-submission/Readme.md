Patrick Gomes
CS 591 Data Science Homework 4

To run the code on a machine, you must download the files Katherine posted and then replace her meta.json with the compressed version I included. I mention it in a comment but I will repeat here just in case, I removed all ' symbols from meta.json so it would parse correctly. All other files are unmodified. I would have included all of the files but they were too large for git.

Report

Rating prediction:

To calculate the rating for a (reviewer,product) pair I break it down into 4 corner cases.

	1) Both the reviewer and product have been seen before
	2) Only the reviewer has been seen and he has reviewed more than 10 products (explained below)
	3) Only the product has been seen and has more than 15 reviews (explained below)
	4) Neither has been seen befire

The fourth case is the least interesting and only predicts the global average rating.

The first case is the most interesting. I create bias' for both reviewer and product. The reviewer bias is based on the ratio of the number of reviews by the reviewer to the number of reviews by the reviewer plus number of reviews the product has. The product bias is based on the ratio of the number of reviews the product has over the same denominator. These values give a good weight to the average rating of the review and product. Finally the prediction value is scaled down and combined with the global average to get a better prediction value.

The second and third cases are based on this idea of weight on activeness/number of reviews. I played around with the numbers a bit and found 10 or 15 to work well. The prediction value here is also scaled with the global average.

The global average scaling was picked after some testing to see what worked best for the most part, changing it anymore than how it is now results in at most minor changes not worth the time. Any major improvements will come from changing the actual logic, not tweaking numbers.

Purchase prediction:

My purchase prediction is based on the baseline method with a few tweaks to improve performance. 

	1) Instead of only the top 50% being taken, I use the top 70%.
	2) I also get the top 80% and with this do the following:
		a) if the product is in the top 80% and has an average rating > 4.5, then say it is purchased
		b) if the product is in the top 80% and the reviewer has bought more than 10 items, then say it is purchased.
This works well because well rated products are more likely to be bought and reviewers who have bought multiple items in the past are more likely to purchase products.

This metric isn't the best it could be and more tweaks could probably be added, such as checking the categories of the product to the previously bought categories of the reviewer, but I couldn't incorporate that in properly without suffering from loss. 

Helpful prediction:

The helpful prediction is done through a dictionary made earlier in the third code block. The dictionary is number of total votes : average number of helpful votes with total votes equal to the key value. This gives me a good quick prediction method by just assuming that the average is always near the actual value for any new total number of votes. 

I also store in spot -1 of the dictionary the global average of helpful votes, but this wasn't useful to do outliers really impacting this value. Maybe with some other techniques I could implement in the average but I didn't feel it would have made a large difference in the end and I was satisfied with the results.
